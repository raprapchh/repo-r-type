#include "Server.hpp"
#include "../shared/net/Protocol.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include "../../ecs/include/systems/MovementSystem.hpp"
#include "../../ecs/include/systems/BoundarySystem.hpp"
#include "../../ecs/include/systems/CollisionSystem.hpp"
#include "../../ecs/include/systems/WeaponSystem.hpp"
#include "../../ecs/include/systems/SpawnSystem.hpp"
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
#include "../../ecs/include/components/Projectile.hpp"
#include "../../ecs/include/components/NetworkId.hpp"
#include "../../ecs/include/components/MapBounds.hpp"
#include "../../ecs/include/components/CollisionLayer.hpp"
#include "../../shared/utils/Logger.hpp"
#include "../../shared/utils/GameConfig.hpp"
#include <unordered_set>

namespace rtype::server {

Server::Server(GameEngine::Registry& registry, uint16_t port)
    : port_(port), next_player_id_(1), running_(false), game_started_(false), registry_(registry) {
    io_context_ = std::make_unique<asio::io_context>();
    udp_server_ = std::make_unique<UdpServer>(*io_context_, port_);
    protocol_adapter_ = std::make_unique<rtype::net::ProtocolAdapter>();
    message_serializer_ = std::make_unique<rtype::net::MessageSerializer>();
}

Server::~Server() {
    stop();
}

void Server::start() {
    if (running_.load()) {
        return;
    }

    running_ = true;
    udp_server_->set_message_handler([this](const std::string& ip, uint16_t port, const std::vector<uint8_t>& data) {
        handle_client_message(ip, port, data);
    });

    // Create enemy spawner entity
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
    }
    Logger::instance().info("Map dimensions: 1920x1080");
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

    while (running_.load()) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_tick);

        // Call of different systems
        if (elapsed >= TICK_DURATION) {
            double dt = elapsed.count() / 1000.0;
            if (dt > 0.1) {
                Logger::instance().warn("Server lag spike detected: " + std::to_string(dt * 1000) + "ms");
            }

            if (game_started_) {
                rtype::ecs::SpawnSystem spawn_system;
                spawn_system.update(registry_, dt);
            }

            // Broadcast newly spawned enemies to clients
            if (protocol_adapter_ && message_serializer_) {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                auto enemy_spawn_view = registry_.view<rtype::ecs::component::Position, rtype::ecs::component::Velocity,
                                                       rtype::ecs::component::Health>();
                enemy_spawn_view.each([&](const auto entity, rtype::ecs::component::Position& pos,
                                          rtype::ecs::component::Velocity& vel, rtype::ecs::component::Health&) {
                    // Skip players (entities with Weapon component)
                    if (registry_.hasComponent<rtype::ecs::component::Weapon>(static_cast<size_t>(entity))) {
                        return;
                    }

                    // Check if enemy already has NetworkId (already broadcasted)
                    if (!registry_.hasComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity))) {
                        // Add NetworkId to mark as broadcasted
                        registry_.addComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity),
                                                                                 static_cast<uint32_t>(entity));

                        // Send EntitySpawn to all clients
                        uint16_t sub_type = 0;
                        if (registry_.hasComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity))) {
                            auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity));
                            if (tag.name == "Monster_0_Top")
                                sub_type = 1;
                            else if (tag.name == "Monster_0_Bot")
                                sub_type = 2;
                            else if (tag.name == "Monster_0_Left")
                                sub_type = 3;
                            else if (tag.name == "Monster_0_Right")
                                sub_type = 4;
                        }

                        rtype::net::EntitySpawnData spawn_data(static_cast<uint32_t>(entity),
                                                               rtype::net::EntityType::ENEMY, sub_type, pos.x, pos.y,
                                                               vel.vx, vel.vy);

                        rtype::net::Packet spawn_packet = message_serializer_->serialize_entity_spawn(spawn_data);
                        auto serialized_spawn = protocol_adapter_->serialize(spawn_packet);

                        for (const auto& [dest_key, dest_client] : clients_) {
                            if (dest_client.is_connected && udp_server_) {
                                udp_server_->send(dest_client.ip, dest_client.port, serialized_spawn);
                            }
                        }

                        Logger::instance().info("Enemy spawned and broadcasted: entity_id=" +
                                                std::to_string(static_cast<uint32_t>(entity)));
                    }
                });
            }

            {
                std::lock_guard<std::mutex> registry_lock(registry_mutex_);
                rtype::ecs::MovementSystem movement_system;
                movement_system.update(registry_, dt);

                // Track enemy entities before BoundarySystem (to detect destroyed ones)
                std::unordered_set<uint32_t> enemies_before;
                {
                    auto enemy_track_view =
                        registry_.view<rtype::ecs::component::Health, rtype::ecs::component::NetworkId>();
                    enemy_track_view.each([&](const auto entity, rtype::ecs::component::Health&,
                                              rtype::ecs::component::NetworkId& net_id) {
                        // Only track enemies (no Weapon component)
                        if (!registry_.hasComponent<rtype::ecs::component::Weapon>(static_cast<size_t>(entity))) {
                            enemies_before.insert(net_id.id);
                        }
                    });
                }

                rtype::ecs::BoundarySystem boundary_system;
                boundary_system.update(registry_, dt);

                rtype::ecs::CollisionSystem collision_system;
                collision_system.update(registry_, dt);

                rtype::ecs::WeaponSystem weapon_system;
                weapon_system.update(registry_, dt);

                // Track enemy entities after systems and find destroyed ones
                if (protocol_adapter_ && message_serializer_) {
                    std::lock_guard<std::mutex> lock(clients_mutex_);

                    std::unordered_set<uint32_t> enemies_after;
                    {
                        auto enemy_track_view =
                            registry_.view<rtype::ecs::component::Health, rtype::ecs::component::NetworkId>();
                        enemy_track_view.each([&](const auto entity, rtype::ecs::component::Health&,
                                                  rtype::ecs::component::NetworkId& net_id) {
                            // Only track enemies (no Weapon component)
                            if (!registry_.hasComponent<rtype::ecs::component::Weapon>(static_cast<size_t>(entity))) {
                                enemies_after.insert(net_id.id);
                            }
                        });
                    }

                    // Find destroyed enemies (in before but not in after)
                    for (uint32_t enemy_id : enemies_before) {
                        if (enemies_after.find(enemy_id) == enemies_after.end()) {
                            // Enemy was destroyed - broadcast to clients
                            rtype::net::EntityDestroyData destroy_data;
                            destroy_data.entity_id = enemy_id;
                            destroy_data.reason = rtype::net::DestroyReason::TIMEOUT;

                            rtype::net::Packet destroy_packet =
                                message_serializer_->serialize_entity_destroy(destroy_data);
                            auto serialized_destroy = protocol_adapter_->serialize(destroy_packet);

                            for (const auto& [dest_key, dest_client] : clients_) {
                                if (dest_client.is_connected && udp_server_) {
                                    udp_server_->send(dest_client.ip, dest_client.port, serialized_destroy);
                                }
                            }

                            Logger::instance().info("Enemy destroyed and broadcasted: entity_id=" +
                                                    std::to_string(enemy_id));
                        }
                    }
                }

                rtype::ecs::ScoreSystem score_system;
                score_system.update(registry_, dt);

                rtype::ecs::LivesSystem lives_system;
                lives_system.update(registry_, dt);

                auto projectile_view =
                    registry_.view<rtype::ecs::component::Projectile, rtype::ecs::component::Position,
                                   rtype::ecs::component::Velocity>();
                for (auto entity : projectile_view) {
                    auto id = static_cast<size_t>(entity);
                    if (!registry_.hasComponent<rtype::ecs::component::NetworkId>(id)) {
                        auto& pos = registry_.getComponent<rtype::ecs::component::Position>(id);
                        auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(id);

                        uint32_t net_id = static_cast<uint32_t>(id);
                        registry_.addComponent<rtype::ecs::component::NetworkId>(id, net_id);

                        uint16_t sub_type = 0;
                        if (registry_.hasComponent<rtype::ecs::component::Tag>(id)) {
                            auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(id);
                            if (tag.name == "Monster_0_Ball")
                                sub_type = 1;
                        }

                        rtype::net::EntitySpawnData spawn_data;
                        spawn_data.entity_id = net_id;
                        spawn_data.entity_type = rtype::net::EntityType::PROJECTILE;
                        spawn_data.sub_type = sub_type;
                        spawn_data.position_x = pos.x;
                        spawn_data.position_y = pos.y;
                        spawn_data.velocity_x = vel.vx;
                        spawn_data.velocity_y = vel.vy;

                        rtype::net::Packet spawn_packet = message_serializer_->serialize_entity_spawn(spawn_data);
                        auto serialized_spawn = protocol_adapter_->serialize(spawn_packet);

                        std::lock_guard<std::mutex> clients_lock(clients_mutex_);
                        for (const auto& [key, client] : clients_) {
                            if (client.is_connected && udp_server_) {
                                udp_server_->send(client.ip, client.port, serialized_spawn);
                            }
                        }
                        Logger::instance().info("Spawned projectile " + std::to_string(net_id));
                    }
                }
            }

            if (protocol_adapter_ && message_serializer_) {
                std::vector<rtype::net::PlayerMoveData> move_data_list;
                {
                    std::lock_guard<std::mutex> registry_lock(registry_mutex_);
                    std::lock_guard<std::mutex> clients_lock(clients_mutex_);
                    for (const auto& [key, client] : clients_) {
                        if (!client.is_connected)
                            continue;

                        if (!registry_.isValid(client.entity_id))
                            continue;

                        if (registry_.hasComponent<rtype::ecs::component::Position>(client.entity_id) &&
                            registry_.hasComponent<rtype::ecs::component::Velocity>(client.entity_id)) {

                            auto& pos = registry_.getComponent<rtype::ecs::component::Position>(client.entity_id);
                            auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(client.entity_id);

                            move_data_list.emplace_back(client.player_id, pos.x, pos.y, vel.vx, vel.vy);
                        }
                    }
                }

                for (const auto& move_data : move_data_list) {
                    rtype::net::Packet move_packet = message_serializer_->serialize_player_move(move_data);
                    auto serialized_move = protocol_adapter_->serialize(move_packet);

                    std::lock_guard<std::mutex> clients_lock(clients_mutex_);
                    for (const auto& [dest_key, dest_client] : clients_) {
                        if (dest_client.is_connected && udp_server_) {
                            udp_server_->send(dest_client.ip, dest_client.port, serialized_move);
                        }
                    }
                }

                // Broadcast enemy positions to all clients
                auto enemy_view = registry_.view<rtype::ecs::component::Position, rtype::ecs::component::Velocity,
                                                 rtype::ecs::component::Health>();
                enemy_view.each([&](const auto entity, rtype::ecs::component::Position& pos,
                                    rtype::ecs::component::Velocity& vel, rtype::ecs::component::Health&) {
                    // Skip players (entities with Weapon component)
                    if (registry_.hasComponent<rtype::ecs::component::Weapon>(static_cast<size_t>(entity))) {
                        return;
                    }

                    rtype::net::EntityMoveData enemy_move_data;
                    enemy_move_data.entity_id = static_cast<uint32_t>(entity);
                    enemy_move_data.position_x = pos.x;
                    enemy_move_data.position_y = pos.y;
                    enemy_move_data.velocity_x = vel.vx;
                    enemy_move_data.velocity_y = vel.vy;

                    rtype::net::Packet enemy_packet = message_serializer_->serialize_entity_move(enemy_move_data);
                    auto serialized_enemy = protocol_adapter_->serialize(enemy_packet);

                    for (const auto& [dest_key, dest_client] : clients_) {
                        if (dest_client.is_connected && udp_server_) {
                            udp_server_->send(dest_client.ip, dest_client.port, serialized_enemy);
                        }
                    }
                });

                rtype::net::GameStateData game_state_data;
                game_state_data.game_time = static_cast<uint32_t>(elapsed.count());
                game_state_data.wave_number = 1;
                game_state_data.enemies_remaining = 0;
                game_state_data.score = 0;
                game_state_data.game_state = 0;
                game_state_data.padding[0] = 0;
                game_state_data.padding[1] = 0;

                {
                    std::lock_guard<std::mutex> registry_lock(registry_mutex_);
                    std::lock_guard<std::mutex> clients_lock(clients_mutex_);
                    for (const auto& [key, client] : clients_) {
                        if (client.is_connected && udp_server_) {
                            if (!registry_.isValid(client.entity_id)) {
                                game_state_data.score = 0;
                                game_state_data.lives = 0;
                            } else {
                                if (registry_.hasComponent<rtype::ecs::component::Score>(client.entity_id)) {
                                    game_state_data.score =
                                        registry_.getComponent<rtype::ecs::component::Score>(client.entity_id).value;
                                } else {
                                    game_state_data.score = 0;
                                }

                                if (registry_.hasComponent<rtype::ecs::component::Lives>(client.entity_id)) {
                                    game_state_data.lives =
                                        registry_.getComponent<rtype::ecs::component::Lives>(client.entity_id)
                                            .remaining;
                                } else {
                                    game_state_data.lives = 0;
                                }
                            }

                            game_state_data.game_state = 0;

                            rtype::net::Packet state_packet =
                                message_serializer_->serialize_game_state(game_state_data);
                            auto packet_data = protocol_adapter_->serialize(state_packet);
                            udp_server_->send(client.ip, client.port, packet_data);
                        }
                    }
                }
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

    default:
        Logger::instance().warn("Unknown message type: " + std::to_string(packet.header.message_type));
    }
}

void Server::handle_player_join(const std::string& client_ip, uint16_t client_port) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    uint32_t player_id;
    GameEngine::entity_t entity;
    {
        std::lock_guard<std::mutex> registry_lock(registry_mutex_);
        std::lock_guard<std::mutex> clients_lock(clients_mutex_);
        if (clients_.find(client_key) != clients_.end()) {
            Logger::instance().warn("Player already connected from " + client_ip + ":" + std::to_string(client_port));
            return;
        }

        player_id = next_player_id_++;

        float start_x = 100.0f + (player_id - 1) * 150.0f;
        float start_y = 100.0f + (player_id - 1) * 100.0f;

        entity = registry_.createEntity();
        registry_.addComponent<rtype::ecs::component::Position>(entity, start_x, start_y);
        registry_.addComponent<rtype::ecs::component::Velocity>(entity, 0.0f, 0.0f);
        registry_.addComponent<rtype::ecs::component::HitBox>(entity, 66.0f, 110.0f);
        auto& weapon = registry_.addComponent<rtype::ecs::component::Weapon>(entity);
        weapon.spawnOffsetX = 35.0f;
        weapon.spawnOffsetY = 10.0f;
        registry_.addComponent<rtype::ecs::component::Score>(entity, 0);
        registry_.addComponent<rtype::ecs::component::Tag>(entity, "Player");

        ClientInfo info;
        info.ip = client_ip;
        info.port = client_port;
        info.player_id = player_id;
        info.is_connected = true;
        info.entity_id = entity;

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
        std::lock_guard<std::mutex> lock(registry_mutex_);

        auto enemy_view = registry_.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position,
                                         rtype::ecs::component::Velocity, rtype::ecs::component::Health>();
        for (auto entity : enemy_view) {
            auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
            auto& pos = registry_.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
            auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(static_cast<size_t>(entity));

            uint16_t sub_type = 0;
            if (registry_.hasComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity))) {
                auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity));
                if (tag.name == "Monster_0_Top")
                    sub_type = 1;
                else if (tag.name == "Monster_0_Bot")
                    sub_type = 2;
                else if (tag.name == "Monster_0_Left")
                    sub_type = 3;
                else if (tag.name == "Monster_0_Right")
                    sub_type = 4;
            }

            rtype::net::EntitySpawnData spawn_data(net_id.id, rtype::net::EntityType::ENEMY, sub_type, pos.x, pos.y,
                                                   vel.vx, vel.vy);
            rtype::net::Packet spawn_packet = message_serializer_->serialize_entity_spawn(spawn_data);
            auto serialized_spawn = protocol_adapter_->serialize(spawn_packet);
            udp_server_->send(client_ip, client_port, serialized_spawn);
        }

        auto projectile_view = registry_.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Projectile,
                                              rtype::ecs::component::Position, rtype::ecs::component::Velocity>();
        for (auto entity : projectile_view) {
            auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
            auto& pos = registry_.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
            auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(static_cast<size_t>(entity));

            uint16_t sub_type = 0;
            if (registry_.hasComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity))) {
                auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity));
                if (tag.name == "Monster_0_Ball")
                    sub_type = 1;
            }

            rtype::net::EntitySpawnData spawn_data;
            spawn_data.entity_id = net_id.id;
            spawn_data.entity_type = rtype::net::EntityType::PROJECTILE;
            spawn_data.sub_type = sub_type;
            spawn_data.position_x = pos.x;
            spawn_data.position_y = pos.y;
            spawn_data.velocity_x = vel.vx;
            spawn_data.velocity_y = vel.vy;

            rtype::net::Packet spawn_packet = message_serializer_->serialize_entity_spawn(spawn_data);
            auto serialized_spawn = protocol_adapter_->serialize(spawn_packet);
            udp_server_->send(client_ip, client_port, serialized_spawn);
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

    Logger::instance().info("Player " + std::to_string(player_id) + " shot");

    try {
        message_serializer_->deserialize_player_shoot(packet);

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
            if (registry_.hasComponent<rtype::ecs::component::Weapon>(entity_id)) {
                auto& weapon = registry_.getComponent<rtype::ecs::component::Weapon>(entity_id);
                weapon.isShooting = true;
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
    game_started_ = true;
    Logger::instance().info("Game started!");
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

} // namespace rtype::server
