#include "Server.hpp"
#include "../../shared/GameConstants.hpp"
#include "../../ecs/include/components/PlayerName.hpp"
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
#include "../../ecs/include/components/MapBounds.hpp"
#include "../../ecs/include/components/CollisionLayer.hpp"
#include "../../shared/utils/Logger.hpp"
#include "../../shared/utils/GameConfig.hpp"
#include <unordered_set>
#include <chrono>
#include <fstream>
#include <string>
#include <vector>

namespace rtype::server {

Server::Server(GameEngine::Registry& registry, uint16_t port)
    : port_(port), next_player_id_(1), running_(false), game_started_(false), game_over_(false), registry_(registry) {
    io_context_ = std::make_unique<asio::io_context>();
    udp_server_ = std::make_unique<UdpServer>(*io_context_, port_);
    protocol_adapter_ = std::make_unique<rtype::net::ProtocolAdapter>();
    message_serializer_ = std::make_unique<rtype::net::MessageSerializer>();
    broadcast_system_ =
        std::make_unique<BroadcastSystem>(registry_, *udp_server_, *protocol_adapter_, *message_serializer_);
    system_manager_.addSystem<rtype::ecs::MovementSystem>();
    system_manager_.addSystem<rtype::ecs::MobSystem>();
    system_manager_.addSystem<rtype::ecs::BoundarySystem>();
    system_manager_.addSystem<rtype::ecs::CollisionSystem>();
    system_manager_.addSystem<rtype::ecs::LivesSystem>();
    system_manager_.addSystem<rtype::ecs::WeaponSystem>();
    system_manager_.addSystem<rtype::ecs::ProjectileSystem>();
    system_manager_.addSystem<rtype::ecs::ScoreSystem>();
}

Server::~Server() {
    game_started_ = false;
}

void load_level(GameEngine::Registry& registry, const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::instance().error("Failed to open map: " + path);
        return;
    }

    static uint32_t next_id = 10000;
    std::string line;
    int row = 0;
    while (std::getline(file, line)) {
        for (size_t col = 0; col < line.length(); ++col) {
            char c = line[col];
            if (c == ' ')
                continue;
            float x = col * 288.0f, y = row * 100.0f;
            bool is_floor = (c == '2');
            if (c == '1' || c == '2' || c == '3' || c == '4') {
                auto e = registry.createEntity();
                registry.addComponent<rtype::ecs::component::Position>(e, x, y);
                float w = (is_floor ? rtype::constants::FLOOR_OBSTACLE_WIDTH : rtype::constants::OBSTACLE_WIDTH) *
                          rtype::constants::OBSTACLE_SCALE;
                float h = (is_floor ? rtype::constants::FLOOR_OBSTACLE_HEIGHT : rtype::constants::OBSTACLE_HEIGHT) *
                          rtype::constants::OBSTACLE_SCALE;
                registry.addComponent<rtype::ecs::component::HitBox>(e, w, h);
                registry.addComponent<rtype::ecs::component::Collidable>(
                    e, rtype::ecs::component::CollisionLayer::Obstacle);
                registry.addComponent<rtype::ecs::component::NetworkId>(e, next_id++);
                registry.addComponent<rtype::ecs::component::Tag>(e, is_floor ? "Obstacle_Floor" : "Obstacle");
                registry.addComponent<rtype::ecs::component::Velocity>(e, -100.0f, 0.0f);
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

    Logger::instance().info("Enemy spawner initialized");
}

void Server::stop() {
    if (!running_.load())
        return;
    running_ = false;
    Logger::instance().info("Server stopping...");
    if (udp_server_)
        udp_server_->stop();
    if (work_guard_.has_value()) {
        work_guard_->reset();
        work_guard_.reset();
    }
    if (io_context_)
        io_context_->stop();
    if (game_thread_.joinable())
        game_thread_.join();
    if (network_thread_.joinable())
        network_thread_.join();
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

            std::lock_guard<std::mutex> registry_lock(registry_mutex_);
            std::lock_guard<std::mutex> clients_lock(clients_mutex_);
            for (const auto& [key, client] : clients_) {
                if (!client.is_connected || !udp_server_)
                    continue;
                state.score = 0;
                state.lives = 0;
                if (registry_.isValid(client.entity_id)) {
                    if (registry_.hasComponent<rtype::ecs::component::Score>(client.entity_id))
                        state.score = registry_.getComponent<rtype::ecs::component::Score>(client.entity_id).value;
                    if (registry_.hasComponent<rtype::ecs::component::Lives>(client.entity_id))
                        state.lives = registry_.getComponent<rtype::ecs::component::Lives>(client.entity_id).remaining;
                }
                udp_server_->send(client.ip, client.port,
                                  protocol_adapter_->serialize(message_serializer_->serialize_game_state(state)));
            }
        }

        void Server::network_loop() {
            if (io_context_) {
                io_context_->run();
            }
        }

        void Server::handle_client_message(const std::string& client_ip, uint16_t client_port,
                                           const std::vector<uint8_t>& data) {
            if (!protocol_adapter_ || !protocol_adapter_->validate(data)) {
                if (protocol_adapter_)
                    Logger::instance().warn("Invalid packet from " + client_ip + ":" + std::to_string(client_port));
                return;
            }
            rtype::net::Packet packet = protocol_adapter_->deserialize(data);
            std::string client_key = client_ip + ":" + std::to_string(client_port);
            {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                auto it = clients_.find(client_key);
                if (it != clients_.end())
                    it->second.last_seen = std::chrono::steady_clock::now();
            }

            switch (static_cast<rtype::net::MessageType>(packet.header.message_type)) {
            case rtype::net::MessageType::PlayerJoin:
                handle_player_join(client_ip, client_port, packet);
                break;
            case rtype::net::MessageType::PlayerName:
                handle_player_name(client_ip, client_port, packet);
                break;
            case rtype::net::MessageType::PlayerMove:
                handle_player_move(client_ip, client_port, packet);
                break;
            case rtype::net::MessageType::PlayerShoot:
                handle_player_shoot(client_ip, client_port, packet);
                break;
            case rtype::net::MessageType::Ping:
                if (message_serializer_) {
                    auto ping = message_serializer_->deserialize_ping_pong(packet);
                    udp_server_->send(client_ip, client_port,
                                      protocol_adapter_->serialize(message_serializer_->serialize_pong(
                                          rtype::net::PingPongData(ping.timestamp))));
                }
                break;
            case rtype::net::MessageType::GameStart:
                handle_game_start(client_ip, client_port, packet);
                break;
            case rtype::net::MessageType::MapResize:
                handle_map_resize(client_ip, client_port, packet);
                break;
            case rtype::net::MessageType::PlayerLeave: {
                ClientInfo info;
                bool found = false;
                {
                    std::lock_guard<std::mutex> lock(clients_mutex_);
                    auto it = clients_.find(client_key);
                    if (it != clients_.end()) {
                        info = it->second;
                        found = true;
                    }
                }
                if (found)
                    disconnect_client(client_key, info);
            } break;
            default:
                Logger::instance().warn("Unknown message type: " + std::to_string(packet.header.message_type));
            }
        }

        void Server::handle_player_join(const std::string& client_ip, uint16_t client_port,
                                        const rtype::net::Packet& packet) {
            if (!running_.load())
                return;

            std::string client_key = client_ip + ":" + std::to_string(client_port);
            std::string player_name = "Player";
            if (message_serializer_) {
                auto join_request = message_serializer_->deserialize_player_join(packet);
                if (join_request.player_name[0] != '\0')
                    player_name = std::string(join_request.player_name);
            }

            // Check if already connected or server full
            {
                std::lock_guard<std::mutex> clients_lock(clients_mutex_);
                if (clients_.find(client_key) != clients_.end()) {
                    if (protocol_adapter_ && message_serializer_ && udp_server_) {
                        rtype::net::PlayerJoinData join_data(clients_[client_key].player_id,
                                                             clients_[client_key].player_name);
                        udp_server_->send(
                            client_ip, client_port,
                            protocol_adapter_->serialize(message_serializer_->serialize_player_join(join_data)));
                    }
                    return;
                }
                size_t connected_count = 0;
                for (const auto& [key, client] : clients_)
                    if (client.is_connected)
                        connected_count++;
                if (connected_count >= rtype::constants::MAX_PLAYERS) {
                    Logger::instance().warn("Server full, rejecting " + client_ip + ":" + std::to_string(client_port));
                    if (protocol_adapter_ && message_serializer_ && udp_server_) {
                        udp_server_->send(client_ip, client_port,
                                          protocol_adapter_->serialize(message_serializer_->serialize_player_join(
                                              rtype::net::PlayerJoinData(0, ""))));
                    }
                    return;
                }
            }

            // Create player entity and register client
            uint32_t player_id;
            GameEngine::entity_t entity;
            {
                std::lock_guard<std::mutex> registry_lock(registry_mutex_);
                std::lock_guard<std::mutex> clients_lock(clients_mutex_);
                player_id = next_player_id_++;
                entity = create_player_entity(player_id, player_name);
                Logger::instance().info("Creating player entity: player_id=" + std::to_string(player_id) +
                                        ", entity_id=" + std::to_string(static_cast<uint32_t>(entity)));
                clients_[client_key] = {
                    client_ip, client_port, player_id, player_name, true, entity, std::chrono::steady_clock::now()};
            }

            Logger::instance().info("Player " + std::to_string(player_id) + " (" + player_name + ") joined from " +
                                    client_ip + ":" + std::to_string(client_port));
            if (!protocol_adapter_ || !message_serializer_)
                return;

            // Send join confirmation
            auto response_data = protocol_adapter_->serialize(
                message_serializer_->serialize_player_join(rtype::net::PlayerJoinData(player_id, player_name)));
            udp_server_->send(client_ip, client_port, response_data);
            broadcast_message(response_data, client_ip, client_port);

            // Notify new player about existing players
            {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                for (const auto& [key, c] : clients_) {
                    if (c.is_connected && c.player_id != player_id) {
                        udp_server_->send(client_ip, client_port,
                                          protocol_adapter_->serialize(message_serializer_->serialize_player_join(
                                              rtype::net::PlayerJoinData(c.player_id, c.player_name))));
                    }
                }
            }

            // Send existing game entities
            {
                std::lock_guard<std::mutex> registry_lock(registry_mutex_);
                if (broadcast_system_) {
                    broadcast_system_->send_initial_state(client_ip, client_port);
                }
            }
        }

        void Server::handle_player_move(const std::string& client_ip, uint16_t client_port,
                                        const rtype::net::Packet& packet) {
            std::string client_key = client_ip + ":" + std::to_string(client_port);
            try {
                auto move_data = message_serializer_->deserialize_player_move(packet);
                GameEngine::entity_t entity_id;
                {
                    std::lock_guard<std::mutex> lock(clients_mutex_);
                    auto it = clients_.find(client_key);
                    if (it == clients_.end() || !it->second.is_connected)
                        return;
                    entity_id = it->second.entity_id;
                }
                {
                    std::lock_guard<std::mutex> lock(registry_mutex_);
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

        void Server::handle_player_shoot(const std::string& client_ip, uint16_t client_port,
                                         const rtype::net::Packet& packet) {
            std::string client_key = client_ip + ":" + std::to_string(client_port);
            uint32_t player_id = 0;
            GameEngine::entity_t entity_id;
            {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                auto it = clients_.find(client_key);
                if (it == clients_.end() || !it->second.is_connected)
                    return;
                player_id = it->second.player_id;
                entity_id = it->second.entity_id;
            }
            if (!protocol_adapter_ || !message_serializer_)
                return;
            try {
                auto shoot_data = message_serializer_->deserialize_player_shoot(packet);
                Logger::instance().info("Player " + std::to_string(player_id) +
                                        " shot, Charge: " + std::to_string(shoot_data.weapon_type));
                std::lock_guard<std::mutex> lock(registry_mutex_);
                if (!registry_.isValid(entity_id))
                    return;
                if (registry_.hasComponent<rtype::ecs::component::Weapon>(entity_id)) {
                    auto& w = registry_.getComponent<rtype::ecs::component::Weapon>(entity_id);
                    w.isShooting = true;
                    w.chargeLevel = shoot_data.weapon_type;
                }
            } catch (const std::exception& e) {
                Logger::instance().error("Error handling player shoot: " + std::string(e.what()));
            }
        }

        void Server::handle_game_start(const std::string& client_ip, uint16_t client_port,
                                       const rtype::net::Packet& packet) {
            (void)client_ip;
            (void)client_port;
            (void)packet;

            if (game_started_.load()) {
                return;
            }

            game_started_ = true;
            Logger::instance().info("Game started!");
            if (!protocol_adapter_ || !message_serializer_)
                return;
            std::lock_guard<std::mutex> lock(clients_mutex_);
            rtype::net::GameStartData start_data{
                1, 1, static_cast<uint8_t>(clients_.size()), 1,
                static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(
                                          std::chrono::system_clock::now().time_since_epoch())
                                          .count())};
            auto serialized = protocol_adapter_->serialize(message_serializer_->serialize_game_start(start_data));
            for (const auto& [key, c] : clients_)
                if (c.is_connected && udp_server_)
                    udp_server_->send(c.ip, c.port, serialized);
            Logger::instance().info("GameStart sent to " + std::to_string(clients_.size()) + " clients");
        }

        void Server::handle_map_resize(const std::string& client_ip, uint16_t client_port,
                                       const rtype::net::Packet& packet) {
            (void)client_ip;
            (void)client_port;
            if (!message_serializer_)
                return;
            try {
                auto d = message_serializer_->deserialize_map_resize(packet);
                std::lock_guard<std::mutex> lock(registry_mutex_);
                for (auto e : registry_.view<rtype::ecs::component::MapBounds>()) {
                    auto& b = registry_.getComponent<rtype::ecs::component::MapBounds>(static_cast<size_t>(e));
                    b.maxX = d.width;
                    b.maxY = d.height;
                    break;
                }
                Logger::instance().info("Map resized to " + std::to_string(d.width) + "x" + std::to_string(d.height));
            } catch (const std::exception& e) {
                Logger::instance().error("Error map resize: " + std::string(e.what()));
            }
        }

        void Server::broadcast_message(const std::vector<uint8_t>& data, const std::string& exclude_ip,
                                       uint16_t exclude_port) {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            for (const auto& [key, c] : clients_)
                if (c.is_connected && (c.ip != exclude_ip || c.port != exclude_port) && udp_server_)
                    udp_server_->send(c.ip, c.port, data);
        }

        void Server::check_client_timeouts() {
            auto now = std::chrono::steady_clock::now();
            std::vector<std::pair<std::string, ClientInfo>> timed_out;
            {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                for (const auto& [key, c] : clients_)
                    if (c.is_connected &&
                        std::chrono::duration_cast<std::chrono::seconds>(now - c.last_seen) >= CLIENT_TIMEOUT_DURATION)
                        timed_out.emplace_back(key, c);
            }
            for (const auto& [key, c] : timed_out)
                disconnect_client(key, c);
        }

        void Server::disconnect_client(const std::string& client_key, const ClientInfo& client) {
            Logger::instance().warn("Client timeout: player_id=" + std::to_string(client.player_id));
            {
                std::lock_guard<std::mutex> lock(registry_mutex_);
                if (registry_.isValid(client.entity_id)) {
                    registry_.destroyEntity(client.entity_id);
                    Logger::instance().info("Removed entity " +
                                            std::to_string(static_cast<uint32_t>(client.entity_id)));
                }
            }
            if (protocol_adapter_ && message_serializer_) {
                auto leave_data = protocol_adapter_->serialize(
                    message_serializer_->serialize_player_leave(rtype::net::PlayerLeaveData(client.player_id)));
                for (int i = 0; i < 3; ++i) {
                    broadcast_message(leave_data, client.ip, client.port);
                    if (i < 2)
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                clients_.erase(client_key);
            }
            Logger::instance().info("Client disconnected: player_id=" + std::to_string(client.player_id));
        }

        void Server::broadcast_to_all_clients(const std::vector<uint8_t>& data) {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            for (const auto& [key, client] : clients_) {
                if (client.is_connected && udp_server_) {
                    udp_server_->send(client.ip, client.port, data);
                }
            }
        }

        void Server::broadcast_entity_destroy(uint32_t entity_id, uint8_t reason) {
            if (!protocol_adapter_ || !message_serializer_)
                return;
            broadcast_to_all_clients(
                protocol_adapter_->serialize(message_serializer_->serialize_entity_destroy({entity_id, reason})));
        }

        std::unordered_set<uint32_t> Server::collect_non_player_network_ids() {
            std::unordered_set<uint32_t> ids;
            registry_.view<rtype::ecs::component::NetworkId>().each([&](const auto entity, auto& net_id) {
                auto id = static_cast<size_t>(entity);
                if (registry_.hasComponent<rtype::ecs::component::Tag>(id) &&
                    registry_.getComponent<rtype::ecs::component::Tag>(id).name == "Player")
                    return;
                ids.insert(net_id.id);
            });
            return ids;
        }

        void Server::broadcast_projectile_spawns() {
            if (!protocol_adapter_ || !message_serializer_)
                return;

            std::vector<std::pair<size_t, uint32_t>> projectiles_to_add;
            std::vector<std::pair<uint32_t, std::vector<uint8_t>>> spawns;

            auto view = registry_.view<rtype::ecs::component::Projectile, rtype::ecs::component::Position,
                                       rtype::ecs::component::Velocity>();
            for (auto entity : view) {
                auto id = static_cast<size_t>(entity);
                if (!registry_.hasComponent<rtype::ecs::component::NetworkId>(id)) {
                    auto& pos = registry_.getComponent<rtype::ecs::component::Position>(id);
                    auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(id);
                    uint32_t net_id = static_cast<uint32_t>(id);
                    projectiles_to_add.emplace_back(id, net_id);
                    uint16_t sub_type =
                        registry_.hasComponent<rtype::ecs::component::Tag>(id)
                            ? get_projectile_subtype(registry_.getComponent<rtype::ecs::component::Tag>(id).name)
                            : 0;
                    rtype::net::EntitySpawnData spawn_data(net_id, rtype::net::EntityType::PROJECTILE, sub_type, pos.x,
                                                           pos.y, vel.vx, vel.vy);
                    spawns.emplace_back(
                        net_id, protocol_adapter_->serialize(message_serializer_->serialize_entity_spawn(spawn_data)));
                }
            }

            for (const auto& [entity_id, net_id] : projectiles_to_add) {
                if (registry_.isValid(entity_id))
                    registry_.addComponent<rtype::ecs::component::NetworkId>(entity_id, net_id);
            }
            for (const auto& [net_id, data] : spawns) {
                broadcast_to_all_clients(data);
                Logger::instance().info("Spawned projectile " + std::to_string(net_id));
            }
        }

        uint16_t Server::get_monster_subtype(const std::string& tag_name) {
            if (tag_name == "Monster_0_Top")
                return 1;
            if (tag_name == "Monster_0_Bot")
                return 2;
            if (tag_name == "Monster_0_Left")
                return 3;
            if (tag_name == "Monster_0_Right")
                return 4;
            return 0;
        }

        uint16_t Server::get_projectile_subtype(const std::string& tag_name) {
            if (tag_name == "Monster_0_Ball")
                return 1;
            if (tag_name == "shot_death-charge1")
                return 10;
            if (tag_name == "shot_death-charge2")
                return 11;
            if (tag_name == "shot_death-charge3")
                return 12;
            if (tag_name == "shot_death-charge4")
                return 13;
            return 0;
        }

        void Server::send_entity_spawn(const std::string& ip, uint16_t port, uint32_t entity_id, uint16_t entity_type,
                                       uint16_t sub_type, float x, float y, float vx, float vy) {
            if (!protocol_adapter_ || !message_serializer_ || !udp_server_)
                return;
            rtype::net::EntitySpawnData spawn_data(entity_id, entity_type, sub_type, x, y, vx, vy);
            auto serialized = protocol_adapter_->serialize(message_serializer_->serialize_entity_spawn(spawn_data));
            udp_server_->send(ip, port, serialized);
        }

        GameEngine::entity_t Server::create_player_entity(uint32_t player_id, const std::string& player_name) {
            float start_x = 100.0f + (player_id - 1) * 150.0f;
            float start_y = 100.0f + (player_id - 1) * 100.0f;
            auto entity = registry_.createEntity();
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
            registry_.addComponent<rtype::ecs::component::PlayerName>(entity, player_name);
            return entity;
        }

        void Server::send_existing_entities_to_client(const std::string& client_ip, uint16_t client_port) {
            // Send obstacles
            auto obstacle_view = registry_.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position,
                                                rtype::ecs::component::Tag>();
            for (auto entity : obstacle_view) {
                auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity));
                if (tag.name != "Obstacle" && tag.name != "Obstacle_Floor")
                    continue;
                auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
                auto& pos = registry_.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
                float vx = 0, vy = 0;
                if (registry_.hasComponent<rtype::ecs::component::Velocity>(static_cast<size_t>(entity))) {
                    auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(static_cast<size_t>(entity));
                    vx = vel.vx;
                    vy = vel.vy;
                }
                send_entity_spawn(client_ip, client_port, net_id.id, rtype::net::EntityType::OBSTACLE,
                                  (tag.name == "Obstacle_Floor") ? 1 : 0, pos.x, pos.y, vx, vy);
            }

            // Send enemies
            auto enemy_view = registry_.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position,
                                             rtype::ecs::component::Velocity, rtype::ecs::component::Health>();
            for (auto entity : enemy_view) {
                if (registry_.hasComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity))) {
                    auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity));
                    if (tag.name == "Player")
                        continue;
                    auto& net_id =
                        registry_.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
                    auto& pos = registry_.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
                    auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(static_cast<size_t>(entity));
                    send_entity_spawn(client_ip, client_port, net_id.id, rtype::net::EntityType::ENEMY,
                                      get_monster_subtype(tag.name), pos.x, pos.y, vel.vx, vel.vy);
                }
            }

            // Send projectiles
            auto proj_view = registry_.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Projectile,
                                            rtype::ecs::component::Position, rtype::ecs::component::Velocity>();
            for (auto entity : proj_view) {
                auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
                auto& pos = registry_.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
                auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(static_cast<size_t>(entity));
                uint16_t sub_type = 0;
                if (registry_.hasComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity))) {
                    auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity));
                    sub_type = get_projectile_subtype(tag.name);
                }
                send_entity_spawn(client_ip, client_port, net_id.id, rtype::net::EntityType::PROJECTILE, sub_type,
                                  pos.x, pos.y, vel.vx, vel.vy);
            }
        }
    } // namespace rtype::server
