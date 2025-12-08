#include "Server.hpp"
#include "../shared/net/Protocol.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include "../../ecs/include/systems/MovementSystem.hpp"
#include "../../ecs/include/systems/BoundarySystem.hpp"
#include "../../ecs/include/systems/CollisionSystem.hpp"
#include "../../ecs/include/systems/WeaponSystem.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Velocity.hpp"
#include "../../ecs/include/components/Weapon.hpp"
#include "../../ecs/include/components/HitBox.hpp"
#include "../../shared/utils/Logger.hpp"

namespace rtype::server {

Server::Server(GameEngine::Registry& registry, uint16_t port)
    : port_(port), next_player_id_(1), running_(false), registry_(registry) {
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

    udp_server_->start();
    Logger::instance().info("Server started on port " + std::to_string(port_));
    Logger::instance().info("Map dimensions: 1920x1080");
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

            rtype::ecs::MovementSystem movement_system;
            movement_system.update(registry_, dt);

            rtype::ecs::BoundarySystem boundary_system;
            boundary_system.update(registry_, dt);

            rtype::ecs::CollisionSystem collision_system;
            collision_system.update(registry_, dt);

            rtype::ecs::WeaponSystem weapon_system;
            weapon_system.update(registry_, dt);

            if (protocol_adapter_ && message_serializer_) {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                for (const auto& [key, client] : clients_) {
                    if (!client.is_connected)
                        continue;

                    if (registry_.hasComponent<rtype::ecs::component::Position>(client.entity_id) &&
                        registry_.hasComponent<rtype::ecs::component::Velocity>(client.entity_id)) {

                        auto& pos = registry_.getComponent<rtype::ecs::component::Position>(client.entity_id);
                        auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(client.entity_id);

                        rtype::net::PlayerMoveData move_data(client.player_id, pos.x, pos.y, vel.vx, vel.vy);

                        rtype::net::Packet move_packet = message_serializer_->serialize_player_move(move_data);
                        auto serialized_move = protocol_adapter_->serialize(move_packet);

                        for (const auto& [dest_key, dest_client] : clients_) {
                            if (dest_client.is_connected && udp_server_) {
                                udp_server_->send(dest_client.ip, dest_client.port, serialized_move);
                            }
                        }
                    }
                }

                rtype::net::GameStateData game_state_data;
                game_state_data.game_time = static_cast<uint32_t>(elapsed.count());
                game_state_data.wave_number = 1;
                game_state_data.enemies_remaining = 0;
                game_state_data.score = 0;
                game_state_data.game_state = 0;
                game_state_data.padding[0] = 0;
                game_state_data.padding[1] = 0;
                game_state_data.padding[2] = 0;

                rtype::net::Packet state_packet = message_serializer_->serialize_game_state(game_state_data);
                auto packet_data = protocol_adapter_->serialize(state_packet);

                for (const auto& [key, client] : clients_) {
                    if (client.is_connected && udp_server_) {
                        udp_server_->send(client.ip, client.port, packet_data);
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

    default:
        Logger::instance().warn("Unknown message type: " + std::to_string(packet.header.message_type));
    }
}

void Server::handle_player_join(const std::string& client_ip, uint16_t client_port) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    uint32_t player_id;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        if (clients_.find(client_key) != clients_.end()) {
            Logger::instance().warn("Player already connected from " + client_ip + ":" + std::to_string(client_port));
            return;
        }

        player_id = next_player_id_++;

        float start_x = 100.0f + (player_id - 1) * 150.0f;
        float start_y = 100.0f + (player_id - 1) * 100.0f;

        GameEngine::entity_t entity = registry_.createEntity();
        registry_.addComponent<rtype::ecs::component::Position>(entity, start_x, start_y);
        registry_.addComponent<rtype::ecs::component::Velocity>(entity, 0.0f, 0.0f);
        registry_.addComponent<rtype::ecs::component::HitBox>(entity, 66.0f, 110.0f);
        registry_.addComponent<rtype::ecs::component::Weapon>(entity);

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
}

void Server::handle_player_move(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    try {
        rtype::net::PlayerMoveData move_data = message_serializer_->deserialize_player_move(packet);

        GameEngine::entity_t entity_id;
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            auto it = clients_.find(client_key);
            if (it == clients_.end() || !it->second.is_connected) {
                return;
            }
            entity_id = it->second.entity_id;
        }

        if (registry_.hasComponent<rtype::ecs::component::Velocity>(entity_id)) {
            auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(entity_id);
            vel.vx = move_data.velocity_x;
            vel.vy = move_data.velocity_y;
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
        // rtype::net::PlayerShootData shoot_data = message_serializer_->deserialize_player_shoot(packet);
        auto serialized_packet = protocol_adapter_->serialize(packet);
        broadcast_message(serialized_packet, client_ip, client_port);
    } catch (const std::exception& e) {
        Logger::instance().error("Error handling player shoot: " + std::string(e.what()));
    }
}

void Server::handle_game_start(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(client_key);
        if (it == clients_.end() || !it->second.is_connected) {
            return;
        }
    }

    if (!protocol_adapter_ || !message_serializer_) {
        return;
    }

    try {
        auto start_data = message_serializer_->deserialize_game_start(packet);
        uint8_t connected_count = 0;
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            for (const auto& [key, client] : clients_) {
                if (client.is_connected) {
                    connected_count++;
                }
            }
        }

        if (connected_count >= 2) {
            start_data.player_count = connected_count;
            rtype::net::Packet start_packet = message_serializer_->serialize_game_start(start_data);
            auto serialized_packet = protocol_adapter_->serialize(start_packet);
            {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                for (const auto& [key, client] : clients_) {
                    if (client.is_connected && udp_server_) {
                        udp_server_->send(client.ip, client.port, serialized_packet);
                    }
                }
            }
            Logger::instance().info("Game started with " + std::to_string(connected_count) + " players");
        } else {
            Logger::instance().warn("Game start requested but only " + std::to_string(connected_count) +
                                    " players connected");
        }
    } catch (const std::exception& e) {
        Logger::instance().error("Error handling game start: " + std::string(e.what()));
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
