#include "Client.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include "../../ecs/include/components/NetworkId.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Velocity.hpp"
#include "../../ecs/include/components/Drawable.hpp"
#include "../../ecs/include/components/Controllable.hpp"
#include <iostream>
#include <chrono>

namespace rtype::client {

Client::Client(const std::string& host, uint16_t port, Renderer& renderer)
    : host_(host), port_(port), connected_(false), player_id_(0), renderer_(renderer), network_system_(0) {
    io_context_ = std::make_unique<asio::io_context>();
    udp_client_ = std::make_unique<UdpClient>(*io_context_, host_, port_);
    udp_client_->set_message_handler(
        [this](const asio::error_code& error, std::size_t bytes_transferred, const std::vector<uint8_t>& data) {
            handle_udp_receive(error, bytes_transferred, data);
        });
}

void Client::set_game_start_callback(GameStartCallback callback) {
    game_start_callback_ = callback;
}

void Client::set_player_join_callback(PlayerJoinCallback callback) {
    player_join_callback_ = callback;
}

Client::~Client() {
    disconnect();
}

void Client::connect() {
    asio::ip::udp::resolver resolver(*io_context_);
    rtype::net::MessageSerializer serializer;
    rtype::net::PlayerJoinData join_data;
    rtype::net::Packet join_packet = serializer.serialize_player_join(join_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(join_packet);

    udp_client_->send(packet_data);
    std::cout << "Connection request sent to " << host_ << ":" << port_ << std::endl;

    udp_client_->start_receive();
    run();
}

void Client::disconnect() {
    bool was_connected = connected_.load();
    connected_ = false;

    if (io_context_) {
        io_context_->stop();
    }

    if (network_thread_ && network_thread_->joinable()) {
        network_thread_->join();
    }

    if (udp_client_) {
        udp_client_->stop();
    }

    if (was_connected) {
        std::cout << "Disconnected from server." << std::endl;
    }
}

void Client::run() {
    if (!network_thread_ || !network_thread_->joinable()) {
        network_thread_ = std::make_unique<std::thread>([this]() { io_context_->run(); });
    }
}

void Client::handle_udp_receive(const asio::error_code& error, std::size_t bytes_transferred,
                                const std::vector<uint8_t>& data) {
    if (!error) {
        std::cout << "Received UDP packet size: " << data.size() << " bytes" << std::endl;
        handle_server_message(data);
    } else {
        std::cerr << "Receive error: " << error.message() << std::endl;
        if (error != asio::error::operation_aborted) {
            disconnect();
        }
    }
}

void Client::handle_server_message(const std::vector<uint8_t>& data) {
    rtype::net::ProtocolAdapter adapter;
    if (!adapter.validate(data)) {
        std::cerr << "Error: Invalid packet received (protocol validation failed)." << std::endl;
        return;
    }
    rtype::net::Packet packet = adapter.deserialize(data);
    rtype::net::MessageSerializer serializer;

    std::cout << "Handling message type: " << static_cast<int>(packet.header.message_type)
              << ", Announced body size: " << packet.header.payload_size << ", Actual body size: " << packet.body.size()
              << std::endl;

    if (packet.header.payload_size != packet.body.size()) {
        std::cerr << "Error: Malformed packet received. Announced body size (" << packet.header.payload_size
                  << ") does not match actual body size (" << packet.body.size() << ")." << std::endl;
        return;
    }

    switch (static_cast<rtype::net::MessageType>(packet.header.message_type)) {
    case rtype::net::MessageType::PlayerJoin: {
        try {
            auto join_data = serializer.deserialize_player_join(packet);
            if (!connected_.load()) {
                player_id_ = join_data.player_id;
                network_system_.set_player_id(player_id_);
                connected_ = true;
                std::cout << "Successfully connected to server. My Player ID is " << player_id_ << std::endl;

                std::lock_guard<std::mutex> lock(registry_mutex_);
                auto entity = registry_.createEntity();
                registry_.addComponent<rtype::ecs::component::NetworkId>(entity, player_id_);
                registry_.addComponent<rtype::ecs::component::Position>(entity, 100.0f, 100.0f);
                registry_.addComponent<rtype::ecs::component::Velocity>(entity, 0.0f, 0.0f);
                uint32_t sprite_index = (player_id_ - 1) % 4;
                registry_.addComponent<rtype::ecs::component::Drawable>(entity, "player_ships", sprite_index, 0, 2.0f,
                                                                        2.0f);
                registry_.addComponent<rtype::ecs::component::Controllable>(entity, true);
            } else {
                std::cout << "Player " << join_data.player_id << " has joined the game." << std::endl;

                if (join_data.player_id != player_id_) {
                    std::lock_guard<std::mutex> lock(registry_mutex_);
                    auto entity = registry_.createEntity();
                    registry_.addComponent<rtype::ecs::component::NetworkId>(entity, join_data.player_id);
                    registry_.addComponent<rtype::ecs::component::Position>(entity, 100.0f, 100.0f);
                    registry_.addComponent<rtype::ecs::component::Velocity>(entity, 0.0f, 0.0f);
                    uint32_t sprite_index = (join_data.player_id - 1) % 4;
                    registry_.addComponent<rtype::ecs::component::Drawable>(entity, "player_ships", sprite_index, 0,
                                                                            2.0f, 2.0f);
                }

                if (player_join_callback_) {
                    player_join_callback_(join_data.player_id);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing PlayerJoin packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::GameStart: {
        try {
            auto game_start_data = serializer.deserialize_game_start(packet);
            std::cout << "Game starting! Session: " << game_start_data.session_id
                      << ", Players: " << static_cast<int>(game_start_data.player_count) << std::endl;
            if (game_start_callback_) {
                game_start_callback_();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing GameStart packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::GameState: {
        try {
            auto game_state_data = serializer.deserialize_game_state(packet);
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing GameState packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::PlayerMove: {
        network_system_.push_packet(packet);
        try {
            auto move_data = serializer.deserialize_player_move(packet);

            if (move_data.player_id == player_id_) {
                return;
            }

            std::lock_guard<std::mutex> lock(registry_mutex_);

            bool found = false;
            GameEngine::entity_t found_entity_id = 0;

            {
                auto view = registry_.view<rtype::ecs::component::NetworkId>();
                for (auto entity : view) {
                    GameEngine::entity_t entity_id = static_cast<GameEngine::entity_t>(entity);
                    try {
                        auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(entity_id);
                        if (net_id.id == move_data.player_id) {
                            found_entity_id = entity_id;
                            found = true;
                            break;
                        }
                    } catch (const std::exception& e) {
                        continue;
                    }
                }
            }

            if (found) {
                try {
                    if (registry_.hasComponent<rtype::ecs::component::Position>(found_entity_id)) {
                        auto& pos = registry_.getComponent<rtype::ecs::component::Position>(found_entity_id);
                        pos.x = move_data.position_x;
                        pos.y = move_data.position_y;
                    }
                    if (registry_.hasComponent<rtype::ecs::component::Velocity>(found_entity_id)) {
                        auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(found_entity_id);
                        vel.vx = move_data.velocity_x;
                        vel.vy = move_data.velocity_y;
                    }
                } catch (const std::exception& e) {
                }
            } else {
                try {
                    auto entity = registry_.createEntity();
                    registry_.addComponent<rtype::ecs::component::NetworkId>(entity, move_data.player_id);
                    registry_.addComponent<rtype::ecs::component::Position>(entity, move_data.position_x,
                                                                            move_data.position_y);
                    registry_.addComponent<rtype::ecs::component::Velocity>(entity, move_data.velocity_x,
                                                                            move_data.velocity_y);
                    uint32_t sprite_index = (move_data.player_id - 1) % 4;
                    registry_.addComponent<rtype::ecs::component::Drawable>(entity, "player_ships", sprite_index, 0,
                                                                            2.0f, 2.0f);
                } catch (const std::exception& e) {
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing PlayerMove packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::EntitySpawn: {
        network_system_.push_packet(packet);
        break;
    }

    case rtype::net::MessageType::EntityMove: {
        network_system_.push_packet(packet);
        break;
    }

    case rtype::net::MessageType::EntityDestroy: {
        network_system_.push_packet(packet);
        break;
    }

    case rtype::net::MessageType::Pong: {
        try {
            auto pong_data = serializer.deserialize_ping_pong(packet);
            std::cout << "Received Pong." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing Pong packet: " << e.what() << std::endl;
        }
        break;
    }
    default:
        std::cerr << "Warning: Unknown message type received: " << static_cast<int>(packet.header.message_type)
                  << std::endl;
        break;
    }
}

void Client::send_move(float vx, float vy) {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::PlayerMoveData move_data;
    move_data.player_id = player_id_;
    move_data.position_x = 0.0f;
    move_data.position_y = 0.0f;
    move_data.velocity_x = vx;
    move_data.velocity_y = vy;
    rtype::net::Packet move_packet = serializer.serialize_player_move(move_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(move_packet);
    udp_client_->send(packet_data);
}

void Client::send_shoot(int32_t x, int32_t y) {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::PlayerShootData shoot_data;
    shoot_data.player_id = player_id_;

    float pos_x = 0.0f;
    float pos_y = 0.0f;
    {
        std::lock_guard<std::mutex> lock(registry_mutex_);
        auto view = registry_.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position>();
        for (auto entity : view) {
            auto& net_id =
                registry_.getComponent<rtype::ecs::component::NetworkId>(static_cast<GameEngine::entity_t>(entity));
            if (net_id.id == player_id_) {
                auto& pos =
                    registry_.getComponent<rtype::ecs::component::Position>(static_cast<GameEngine::entity_t>(entity));
                pos_x = pos.x;
                pos_y = pos.y;
                break;
            }
        }
    }

    shoot_data.weapon_type = 0;
    shoot_data.position_x = pos_x;
    shoot_data.position_y = pos_y;
    shoot_data.direction_x = 1.0f;
    shoot_data.direction_y = 0.0f;
    rtype::net::Packet shoot_packet = serializer.serialize_player_shoot(shoot_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(shoot_packet);
    udp_client_->send(packet_data);
}

void Client::send_game_start_request() {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::GameStartData start_data;
    start_data.session_id = 1;
    start_data.level_id = 1;
    start_data.player_count = 0;
    start_data.difficulty = 1;
    start_data.timestamp = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    rtype::net::Packet start_packet = serializer.serialize_game_start(start_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(start_packet);
    udp_client_->send(packet_data);
}

void Client::update() {
    network_system_.update(registry_, registry_mutex_);
}

} // namespace rtype::client
