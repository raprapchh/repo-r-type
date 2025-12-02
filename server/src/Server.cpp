#include "Server.hpp"
#include "../shared/net/Protocol.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include "../../ecs/include/systems/MovementSystem.hpp"
#include "../../ecs/include/systems/BoundarySystem.hpp"
#include "../../ecs/include/systems/CollisionSystem.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Velocity.hpp"

#include <iostream>

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
    std::cout << "Server started on port " << port_ << std::endl;
}

void Server::stop() {
    if (!running_.load()) {
        return;
    }

    running_ = false;

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

            rtype::ecs::MovementSystem movement_system;
            movement_system.update(registry_, dt);

            rtype::ecs::BoundarySystem boundary_system;
            boundary_system.update(registry_, dt);

            rtype::ecs::CollisionSystem collision_system;
            collision_system.update(registry_, dt);

            if (protocol_adapter_) {
                rtype::net::Serializer serializer;
                rtype::net::Packet state_packet(static_cast<uint16_t>(rtype::net::MessageType::GameState),
                                                serializer.get_data());
                auto packet_data = protocol_adapter_->serialize(state_packet);

                std::lock_guard<std::mutex> lock(clients_mutex_);
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

    default:
        std::cout << "Unknown message type: " << packet.header.message_type << std::endl;
    }
}

void Server::handle_player_join(const std::string& client_ip, uint16_t client_port) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    uint32_t player_id;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        if (clients_.find(client_key) != clients_.end()) {
            return;
        }

        player_id = next_player_id_++;

        GameEngine::entity_t entity = registry_.createEntity();
        registry_.addComponent<rtype::ecs::component::Position>(entity, 100.0f, 100.0f);
        registry_.addComponent<rtype::ecs::component::Velocity>(entity, 0.0f, 0.0f);

        ClientInfo info;
        info.ip = client_ip;
        info.port = client_port;
        info.player_id = player_id;
        info.is_connected = true;
        info.entity_id = entity;

        clients_[client_key] = info;
    }

    std::cout << "Player " << player_id << " joined from " << client_ip << ":" << client_port << std::endl;

    if (!protocol_adapter_ || !message_serializer_) {
        return;
    }

    rtype::net::PlayerJoinData join_data(player_id);
    rtype::net::Packet response = message_serializer_->serialize_player_join(join_data);

    auto response_data = protocol_adapter_->serialize(response);
    udp_server_->send(client_ip, client_port, response_data);
    broadcast_message(response_data, client_ip, client_port);
}

void Server::handle_player_move(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    bool is_connected = false;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(client_key);
        if (it == clients_.end() || !it->second.is_connected) {
            return;
        }
        is_connected = true;
    }

    if (!is_connected || !protocol_adapter_ || !message_serializer_) {
        return;
    }

    try {
        rtype::net::PlayerMoveData move_data = message_serializer_->deserialize_player_move(packet);
        auto serialized_packet = protocol_adapter_->serialize(packet);
        broadcast_message(serialized_packet, client_ip, client_port);
    } catch (const std::exception& e) {
        std::cerr << "Error handling player move: " << e.what() << std::endl;
    }
}

void Server::handle_player_shoot(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    bool is_connected = false;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(client_key);
        if (it == clients_.end() || !it->second.is_connected) {
            return;
        }
        is_connected = true;
    }

    if (!is_connected || !protocol_adapter_ || !message_serializer_) {
        return;
    }

    try {
        rtype::net::PlayerShootData shoot_data = message_serializer_->deserialize_player_shoot(packet);
        auto serialized_packet = protocol_adapter_->serialize(packet);
        broadcast_message(serialized_packet, client_ip, client_port);
    } catch (const std::exception& e) {
        std::cerr << "Error handling player shoot: " << e.what() << std::endl;
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
