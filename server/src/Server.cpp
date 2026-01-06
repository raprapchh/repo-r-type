#include "Server.hpp"
#include "../../shared/GameConstants.hpp"
#include "../shared/net/Protocol.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include "../include/BroadcastSystem.hpp"
#include "../../ecs/include/systems/MovementSystem.hpp"
#include "../../ecs/include/systems/BoundarySystem.hpp"
#include "../../ecs/include/systems/CollisionSystem.hpp"
#include "../../ecs/include/systems/WeaponSystem.hpp"
#include "../../ecs/include/systems/SpawnSystem.hpp"
#include "../../ecs/include/systems/MobSystem.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Velocity.hpp"
#include "../../ecs/include/components/Weapon.hpp"
#include "../../ecs/include/components/HitBox.hpp"
#include "../../ecs/include/components/Health.hpp"
#include "../../ecs/include/components/NetworkId.hpp"
#include "../../ecs/include/components/EnemySpawner.hpp"
#include "../../ecs/include/components/Score.hpp"
#include "../../ecs/include/components/Tag.hpp"
#include "../../ecs/include/components/Lives.hpp"
#include "../../ecs/include/systems/ScoreSystem.hpp"
#include "../../ecs/include/systems/LivesSystem.hpp"
#include "../../ecs/include/systems/ProjectileSystem.hpp"
#include "../../ecs/include/components/Projectile.hpp"
#include "../../ecs/include/components/NetworkId.hpp"
#include "../../ecs/include/components/MapBounds.hpp"
#include "../../ecs/include/components/CollisionLayer.hpp"
#include "../../shared/utils/Logger.hpp"
#include "../../shared/utils/GameConfig.hpp"
#include <unordered_set>
#include <chrono>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

namespace rtype::server {

Server::Server(GameEngine::Registry& registry, uint16_t port)
    : port_(port), next_player_id_(1), running_(false), game_started_(false), game_over_(false), registry_(registry) {
    io_context_ = std::make_unique<asio::io_context>();
    udp_server_ = std::make_unique<UdpServer>(*io_context_, port_);
    protocol_adapter_ = std::make_unique<rtype::net::ProtocolAdapter>();
    message_serializer_ = std::make_unique<rtype::net::MessageSerializer>();
    broadcast_system_ =
        std::make_unique<BroadcastSystem>(registry_, *udp_server_, *protocol_adapter_, *message_serializer_);
}

Server::~Server() {
    game_started_ = false;
}

void load_level(GameEngine::Registry& registry, const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::instance().error("Failed to open map file: " + path);
        return;
    }

    std::string line;
    float cell_width = 288.0f;
    float cell_height = 100.0f;

    static uint32_t next_obstacle_network_id = 10000;

    int row = 0;
    while (std::getline(file, line)) {
        for (size_t col = 0; col < line.length(); ++col) {
            char c = line[col];
            if (c == ' ')
                continue;

            float x = col * cell_width;
            float y = row * cell_height;

            if (c == '1' || c == '3' || c == '4') {
                auto obstacle = registry.createEntity();
                registry.addComponent<rtype::ecs::component::Position>(obstacle, x, y);
                registry.addComponent<rtype::ecs::component::HitBox>(
                    obstacle, rtype::constants::OBSTACLE_WIDTH * rtype::constants::OBSTACLE_SCALE,
                    rtype::constants::OBSTACLE_HEIGHT * rtype::constants::OBSTACLE_SCALE);
                registry.addComponent<rtype::ecs::component::Collidable>(
                    obstacle, rtype::ecs::component::CollisionLayer::Obstacle);
                registry.addComponent<rtype::ecs::component::NetworkId>(obstacle, next_obstacle_network_id++);
                registry.addComponent<rtype::ecs::component::Tag>(obstacle, "Obstacle");
                registry.addComponent<rtype::ecs::component::Velocity>(obstacle, -100.0f, 0.0f);
            } else if (c == '2') {
                auto floor_obstacle = registry.createEntity();
                registry.addComponent<rtype::ecs::component::Position>(floor_obstacle, x, y);
                registry.addComponent<rtype::ecs::component::HitBox>(
                    floor_obstacle, rtype::constants::FLOOR_OBSTACLE_WIDTH * rtype::constants::OBSTACLE_SCALE,
                    rtype::constants::FLOOR_OBSTACLE_HEIGHT * rtype::constants::OBSTACLE_SCALE);
                registry.addComponent<rtype::ecs::component::Collidable>(
                    floor_obstacle, rtype::ecs::component::CollisionLayer::Obstacle);
                registry.addComponent<rtype::ecs::component::NetworkId>(floor_obstacle, next_obstacle_network_id++);
                registry.addComponent<rtype::ecs::component::Tag>(floor_obstacle, "Obstacle_Floor");
                registry.addComponent<rtype::ecs::component::Velocity>(floor_obstacle, -100.0f, 0.0f);
            }
        }
        row++;
    }
    Logger::instance().info("Level loaded from " + path);
}

void Server::start() {
    if (running_.load()) {
        return;
    }

    running_ = true;
    udp_server_->set_message_handler([this](const std::string& ip, uint16_t port, const std::vector<uint8_t>& data) {
        handle_client_message(ip, port, data);
    });

    auto spawner = registry_.createEntity();
    registry_.addComponent<rtype::ecs::component::EnemySpawner>(spawner, 2.0f, 0.0f);

    udp_server_->start();
    Logger::instance().info("Server started on port " + std::to_string(port_));

    {
        std::lock_guard<std::mutex> registry_lock(registry_mutex_);
        auto boundsEntity = registry_.createEntity();
        registry_.addComponent<rtype::ecs::component::MapBounds>(boundsEntity, rtype::config::MAP_MIN_X,
                                                                 rtype::config::MAP_MIN_Y, rtype::config::MAP_MAX_X,
                                                                 rtype::config::MAP_MAX_Y);
        Logger::instance().info("Map dimensions: 1920x1080");

        load_level(registry_, "server/assets/map.txt");
    }
    Logger::instance().info("Enemy spawner initialized (interval: 2.0s)");
}

void Server::stop() {
    if (!running_.load()) {
        return;
    }

    running_ = false;
    Logger::instance().info("Server stopping...");

    if (udp_server_) {
        udp_server_->stop();
    }

    if (work_guard_.has_value()) {
        work_guard_->reset();
        work_guard_.reset();
    }

    if (io_context_) {
        io_context_->stop();
    }

    if (game_thread_.joinable()) {
        game_thread_.join();
    }

    if (network_thread_.joinable()) {
        network_thread_.join();
    }
}

void Server::run() {
    start();

    work_guard_.emplace(asio::make_work_guard(*io_context_));

    game_thread_ = std::thread(&Server::game_loop, this);
    network_thread_ = std::thread(&Server::network_loop, this);

    game_thread_.join();
    network_thread_.join();
}

void Server::game_loop() {
    auto last_tick = std::chrono::steady_clock::now();
    auto last_timeout_check = std::chrono::steady_clock::now();

    while (running_.load()) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_tick);

        auto timeout_check_elapsed =
            std::chrono::duration_cast<std::chrono::seconds>(current_time - last_timeout_check);
        if (timeout_check_elapsed >= std::chrono::seconds(1)) {
            check_client_timeouts();
            last_timeout_check = current_time;
        }

        if (elapsed >= TICK_DURATION) {
            double dt = elapsed.count() / 1000.0;
            if (dt > 0.1) {
                Logger::instance().warn("Server lag spike detected: " + std::to_string(dt * 1000) + "ms");
            }

            if (game_started_ && !game_over_.load()) {
                bool all_players_dead = true;
                bool has_players = false;
                {
                    std::lock_guard<std::mutex> clients_lock(clients_mutex_);
                    std::lock_guard<std::mutex> registry_lock(registry_mutex_);
                    for (const auto& [key, client] : clients_) {
                        if (client.is_connected) {
                            has_players = true;
                            if (registry_.isValid(client.entity_id)) {
                                if (registry_.hasComponent<rtype::ecs::component::Lives>(client.entity_id)) {
                                    auto& lives =
                                        registry_.getComponent<rtype::ecs::component::Lives>(client.entity_id);
                                    if (lives.remaining > 0) {
                                        all_players_dead = false;
                                        break;
                                    }
                                } else {
                                    all_players_dead = false;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (has_players && all_players_dead) {
                    game_over_ = true;
                    Logger::instance().info("Game Over - All players are dead. Destroying game entities.");

                    std::vector<GameEngine::entity_t> entities_to_destroy;

                    {
                        std::lock_guard<std::mutex> registry_lock(registry_mutex_);
                        auto network_view = registry_.view<rtype::ecs::component::NetworkId>();
                        for (auto entity : network_view) {
                            bool should_destroy = true;

                            if (registry_.hasComponent<rtype::ecs::component::MapBounds>(static_cast<size_t>(entity))) {
                                should_destroy = false;
                            }
                            if (registry_.hasComponent<rtype::ecs::component::EnemySpawner>(
                                    static_cast<size_t>(entity))) {
                                should_destroy = false;
                            }
                            if (registry_.hasComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity))) {
                                auto& tag =
                                    registry_.getComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity));
                                if (tag.name == "Player") {
                                    should_destroy = false;
                                }
                            }

                            if (should_destroy) {
                                entities_to_destroy.push_back(static_cast<GameEngine::entity_t>(entity));
                            }
                        }

                        for (auto entity : entities_to_destroy) {
                            registry_.destroyEntity(entity);
                        }
                    }
                    Logger::instance().info("Destroyed " + std::to_string(entities_to_destroy.size()) +
                                            " game entities.");
                }

                if (!game_over_.load()) {
                    std::lock_guard<std::mutex> registry_lock(registry_mutex_);

                    rtype::ecs::SpawnSystem spawn_system;
                    spawn_system.update(registry_, dt);

                    rtype::ecs::MovementSystem movement_system;
                    movement_system.update(registry_, dt);

                    rtype::ecs::MobSystem mob_system;
                    mob_system.update(registry_, dt);

                    rtype::ecs::BoundarySystem boundary_system;
                    boundary_system.update(registry_, dt);

                    rtype::ecs::CollisionSystem collision_system;
                    collision_system.update(registry_, dt);

                    rtype::ecs::LivesSystem lives_system;
                    lives_system.update(registry_, dt);

                    rtype::ecs::WeaponSystem weapon_system;
                    weapon_system.update(registry_, dt);

                    rtype::ecs::ProjectileSystem projectile_system;
                    projectile_system.update(registry_, dt);

                    rtype::ecs::ScoreSystem score_system;
                    score_system.update(registry_, dt);
                }
            }

            if (broadcast_system_) {
                std::lock_guard<std::mutex> registry_lock(registry_mutex_);
                std::lock_guard<std::mutex> clients_lock(clients_mutex_);
                broadcast_system_->update(dt, clients_);
            }

            last_tick = current_time;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void Server::network_loop() {
    if (io_context_) {
        io_context_->run();
    }
}

void Server::handle_client_message(const std::string& client_ip, uint16_t client_port,
                                   const std::vector<uint8_t>& data) {
    if (!protocol_adapter_) {
        return;
    }

    if (!protocol_adapter_->validate(data)) {
        Logger::instance().warn("Received invalid packet (validation failed) from " + client_ip + ":" +
                                std::to_string(client_port));
        return;
    }

    rtype::net::Packet packet = protocol_adapter_->deserialize(data);

    std::string client_key = client_ip + ":" + std::to_string(client_port);

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(client_key);
        if (it != clients_.end()) {
            it->second.last_seen = std::chrono::steady_clock::now();
        }
    }

    switch (static_cast<rtype::net::MessageType>(packet.header.message_type)) {
    case rtype::net::MessageType::PlayerJoin:
        handle_player_join(client_ip, client_port);
        break;

    case rtype::net::MessageType::PlayerMove:
        handle_player_move(client_ip, client_port, packet);
        break;

    case rtype::net::MessageType::PlayerShoot:
        handle_player_shoot(client_ip, client_port, packet);
        break;

    case rtype::net::MessageType::Ping: {
        if (message_serializer_) {
            auto ping_data = message_serializer_->deserialize_ping_pong(packet);
            rtype::net::PingPongData pong_data(ping_data.timestamp);
            rtype::net::Packet pong_packet = message_serializer_->serialize_pong(pong_data);
            udp_server_->send(client_ip, client_port, protocol_adapter_->serialize(pong_packet));
        }
    } break;

    case rtype::net::MessageType::GameStart: {
        handle_game_start(client_ip, client_port, packet);
    } break;

    case rtype::net::MessageType::MapResize: {
        handle_map_resize(client_ip, client_port, packet);
    } break;

    case rtype::net::MessageType::PlayerLeave: {
        ClientInfo client_info;
        bool found = false;
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            auto it = clients_.find(client_key);
            if (it != clients_.end()) {
                client_info = it->second;
                found = true;
            }
        }
        if (found) {
            disconnect_client(client_key, client_info);
        }
    } break;

    default:
        Logger::instance().warn("Unknown message type: " + std::to_string(packet.header.message_type));
    }
}

void Server::handle_player_join(const std::string& client_ip, uint16_t client_port) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    {
        std::lock_guard<std::mutex> clients_lock(clients_mutex_);
        if (clients_.find(client_key) != clients_.end()) {
            Logger::instance().warn("Player already connected from " + client_ip + ":" + std::to_string(client_port));
            return;
        }

        size_t connected_count = 0;
        for (const auto& [key, client] : clients_) {
            if (client.is_connected) {
                connected_count++;
            }
        }

        if (connected_count >= rtype::constants::MAX_PLAYERS) {
            Logger::instance().warn("Server full: " + std::to_string(connected_count) +
                                    " players already connected. "
                                    "Rejecting connection from " +
                                    client_ip + ":" + std::to_string(client_port));
            if (protocol_adapter_ && message_serializer_ && udp_server_) {
                rtype::net::PlayerJoinData reject_data(0);
                rtype::net::Packet reject_packet = message_serializer_->serialize_player_join(reject_data);
                auto reject_data_serialized = protocol_adapter_->serialize(reject_packet);
                udp_server_->send(client_ip, client_port, reject_data_serialized);
            }
            return;
        }
    }

    uint32_t player_id;
    GameEngine::entity_t entity;
    {
        std::lock_guard<std::mutex> registry_lock(registry_mutex_);
        std::lock_guard<std::mutex> clients_lock(clients_mutex_);
        player_id = next_player_id_++;

        float start_x = 100.0f + (player_id - 1) * 150.0f;
        float start_y = 100.0f + (player_id - 1) * 100.0f;

        entity = registry_.createEntity();
        Logger::instance().info("Creating player entity: player_id=" + std::to_string(player_id) +
                                ", entity_id=" + std::to_string(static_cast<uint32_t>(entity)));
        registry_.addComponent<rtype::ecs::component::Position>(entity, start_x, start_y);
        registry_.addComponent<rtype::ecs::component::Velocity>(entity, 0.0f, 0.0f);
        registry_.addComponent<rtype::ecs::component::HitBox>(
            entity, rtype::constants::PLAYER_WIDTH * rtype::constants::PLAYER_SCALE,
            rtype::constants::PLAYER_HEIGHT * rtype::constants::PLAYER_SCALE);
        auto& weapon = registry_.addComponent<rtype::ecs::component::Weapon>(entity);
        weapon.spawnOffsetX = 35.0f;
        weapon.spawnOffsetY = 10.0f;
        weapon.fireRate = 0.1f;
        weapon.projectileSpeed = 1500.0f;
        registry_.addComponent<rtype::ecs::component::Health>(entity, 100, 100);
        registry_.addComponent<rtype::ecs::component::Score>(entity, 0);
        registry_.addComponent<rtype::ecs::component::Lives>(entity, 3);
        registry_.addComponent<rtype::ecs::component::Tag>(entity, "Player");
        registry_.addComponent<rtype::ecs::component::NetworkId>(entity, player_id);
        registry_.addComponent<rtype::ecs::component::Collidable>(entity,
                                                                  rtype::ecs::component::CollisionLayer::Player);

        ClientInfo info;
        info.ip = client_ip;
        info.port = client_port;
        info.player_id = player_id;
        info.is_connected = true;
        info.entity_id = entity;
        info.last_seen = std::chrono::steady_clock::now();

        clients_[client_key] = info;
    }

    Logger::instance().info("Player " + std::to_string(player_id) + " joined from " + client_ip + ":" +
                            std::to_string(client_port));

    if (!protocol_adapter_ || !message_serializer_) {
        return;
    }

    rtype::net::PlayerJoinData join_data(player_id);
    rtype::net::Packet response = message_serializer_->serialize_player_join(join_data);

    auto response_data = protocol_adapter_->serialize(response);
    udp_server_->send(client_ip, client_port, response_data);
    broadcast_message(response_data, client_ip, client_port);

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const auto& [key, existing_client] : clients_) {
            if (existing_client.is_connected && existing_client.player_id != player_id) {
                rtype::net::PlayerJoinData existing_join_data(existing_client.player_id);
                rtype::net::Packet existing_response = message_serializer_->serialize_player_join(existing_join_data);
                auto existing_response_data = protocol_adapter_->serialize(existing_response);
                udp_server_->send(client_ip, client_port, existing_response_data);
            }
        }
    }

    {
        std::lock_guard<std::mutex> registry_lock(registry_mutex_);
        if (broadcast_system_) {
            broadcast_system_->send_initial_state(client_ip, client_port);
        }
    }
}

void Server::handle_player_move(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    try {
        rtype::net::PlayerMoveData move_data = message_serializer_->deserialize_player_move(packet);

        GameEngine::entity_t entity_id;
        {
            std::lock_guard<std::mutex> clients_lock(clients_mutex_);
            auto it = clients_.find(client_key);
            if (it == clients_.end() || !it->second.is_connected) {
                return;
            }
            entity_id = it->second.entity_id;
        }

        {
            std::lock_guard<std::mutex> registry_lock(registry_mutex_);
            if (registry_.hasComponent<rtype::ecs::component::Velocity>(entity_id)) {
                auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(entity_id);
                vel.vx = move_data.velocity_x;
                vel.vy = move_data.velocity_y;
            }
        }
    } catch (const std::exception& e) {
        Logger::instance().error("Error handling player move: " + std::string(e.what()));
    }
}

void Server::handle_player_shoot(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    bool is_connected = false;
    uint32_t player_id = 0;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(client_key);
        if (it == clients_.end() || !it->second.is_connected) {
            return;
        }
        is_connected = true;
        player_id = it->second.player_id;
    }

    if (!is_connected || !protocol_adapter_ || !message_serializer_) {
        return;
    }

    try {
        auto shoot_data = message_serializer_->deserialize_player_shoot(packet);

        GameEngine::entity_t entity_id;
        {
            std::lock_guard<std::mutex> clients_lock(clients_mutex_);
            auto it = clients_.find(client_key);
            if (it == clients_.end() || !it->second.is_connected) {
                return;
            }
            entity_id = it->second.entity_id;
        }

        Logger::instance().info("Player " + std::to_string(player_id) + " shot (Entity " +
                                std::to_string(static_cast<uint32_t>(entity_id)) +
                                ") Charge: " + std::to_string(shoot_data.weapon_type));

        {
            std::lock_guard<std::mutex> registry_lock(registry_mutex_);
            if (!registry_.isValid(entity_id)) {
                return;
            }
            if (registry_.hasComponent<rtype::ecs::component::Weapon>(entity_id)) {
                auto& weapon = registry_.getComponent<rtype::ecs::component::Weapon>(entity_id);
                weapon.isShooting = true;
                weapon.chargeLevel = shoot_data.weapon_type;
            }
        }

    } catch (const std::exception& e) {
        Logger::instance().error("Error handling player shoot: " + std::string(e.what()));
    }
}

void Server::handle_game_start(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet) {
    (void)client_ip;
    (void)client_port;
    (void)packet;

    if (game_started_.load()) {
        return;
    }

    game_started_ = true;
    Logger::instance().info("Game started!");

    if (!protocol_adapter_ || !message_serializer_) {
        return;
    }

    std::lock_guard<std::mutex> clients_lock(clients_mutex_);
    uint8_t player_count = static_cast<uint8_t>(clients_.size());

    rtype::net::GameStartData start_data;
    start_data.session_id = 1;
    start_data.level_id = 1;
    start_data.player_count = player_count;
    start_data.difficulty = 1;
    start_data.timestamp = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    rtype::net::Packet start_packet = message_serializer_->serialize_game_start(start_data);
    auto serialized_start = protocol_adapter_->serialize(start_packet);

    for (const auto& [key, client] : clients_) {
        if (client.is_connected && udp_server_) {
            udp_server_->send(client.ip, client.port, serialized_start);
        }
    }

    Logger::instance().info("GameStart message sent to all " + std::to_string(player_count) + " clients");
}

void Server::handle_map_resize(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet) {
    (void)client_ip;
    (void)client_port;
    if (!message_serializer_) {
        return;
    }

    try {
        auto resize_data = message_serializer_->deserialize_map_resize(packet);

        {
            std::lock_guard<std::mutex> registry_lock(registry_mutex_);
            auto view = registry_.view<rtype::ecs::component::MapBounds>();
            for (auto entity : view) {
                auto& bounds =
                    registry_.getComponent<rtype::ecs::component::MapBounds>(static_cast<std::size_t>(entity));
                bounds.maxX = resize_data.width;
                bounds.maxY = resize_data.height;
                break;
            }
        }

        Logger::instance().info("Map resized to " + std::to_string(resize_data.width) + "x" +
                                std::to_string(resize_data.height));
    } catch (const std::exception& e) {
        Logger::instance().error("Error handling map resize: " + std::string(e.what()));
    }
}

void Server::broadcast_message(const std::vector<uint8_t>& data, const std::string& exclude_ip, uint16_t exclude_port) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (const auto& [key, client] : clients_) {
        if (client.is_connected && (client.ip != exclude_ip || client.port != exclude_port)) {
            if (udp_server_) {
                udp_server_->send(client.ip, client.port, data);
            }
        }
    }
}

void Server::check_client_timeouts() {
    auto current_time = std::chrono::steady_clock::now();
    std::vector<std::pair<std::string, ClientInfo>> timed_out_clients;

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const auto& [key, client] : clients_) {
            if (client.is_connected) {
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - client.last_seen);
                if (elapsed >= CLIENT_TIMEOUT_DURATION) {
                    timed_out_clients.emplace_back(key, client);
                }
            }
        }
    }

    for (const auto& [key, client] : timed_out_clients) {
        disconnect_client(key, client);
    }
}

void Server::disconnect_client(const std::string& client_key, const ClientInfo& client) {
    Logger::instance().warn("Client timeout detected: player_id=" + std::to_string(client.player_id) + " from " +
                            client.ip + ":" + std::to_string(client.port));

    {
        std::lock_guard<std::mutex> registry_lock(registry_mutex_);
        if (registry_.isValid(client.entity_id)) {
            registry_.destroyEntity(client.entity_id);
            Logger::instance().info("Removed entity " + std::to_string(static_cast<uint32_t>(client.entity_id)) +
                                    " for timed out player " + std::to_string(client.player_id));
        }
    }

    if (protocol_adapter_ && message_serializer_) {
        rtype::net::PlayerLeaveData leave_data(client.player_id);
        rtype::net::Packet leave_packet = message_serializer_->serialize_player_leave(leave_data);
        auto serialized_leave = protocol_adapter_->serialize(leave_packet);

        constexpr int RETRY_COUNT = 3;
        for (int retry = 0; retry < RETRY_COUNT; ++retry) {
            {
                std::lock_guard<std::mutex> clients_lock(clients_mutex_);
                for (const auto& [dest_key, dest_client] : clients_) {
                    if (dest_client.is_connected && dest_key != client_key && udp_server_) {
                        udp_server_->send(dest_client.ip, dest_client.port, serialized_leave);
                    }
                }
            }
            if (retry < RETRY_COUNT - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        Logger::instance().info("PlayerLeave message broadcasted " + std::to_string(RETRY_COUNT) +
                                " times for player " + std::to_string(client.player_id));
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_.erase(client_key);
    }

    Logger::instance().info("Client disconnected: player_id=" + std::to_string(client.player_id));
}

} // namespace rtype::server
