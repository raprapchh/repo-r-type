#include "Client.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include "../../ecs/include/components/NetworkId.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Velocity.hpp"
#include "../../ecs/include/components/Drawable.hpp"
#include "../../ecs/include/components/Controllable.hpp"
#include <iostream>

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
            renderer_.update_game_state(game_state_data);
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing GameState packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::PlayerMove: {
        network_system_.push_packet(packet);
        try {
            auto move_data = serializer.deserialize_player_move(packet);
            rtype::client::Entity moved_entity;
            moved_entity.id = move_data.player_id;
            moved_entity.type = rtype::net::EntityType::PLAYER;
            moved_entity.x = move_data.position_x;
            moved_entity.y = move_data.position_y;
            renderer_.update_entity(moved_entity);
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing PlayerMove packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::EntitySpawn: {
        network_system_.push_packet(packet);
        try {
            auto spawn_data = serializer.deserialize_entity_spawn(packet);
            rtype::client::Entity new_entity;
            new_entity.id = spawn_data.entity_id;
            new_entity.type = spawn_data.entity_type;
            new_entity.x = spawn_data.position_x;
            new_entity.y = spawn_data.position_y;
            new_entity.velocity_x = spawn_data.velocity_x;
            new_entity.velocity_y = spawn_data.velocity_y;
            renderer_.spawn_entity(new_entity);
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing EntitySpawn packet: " << e.what() << std::endl;
        }
        break;
    }

    case rtype::net::MessageType::EntityDestroy: {
        network_system_.push_packet(packet);
        try {
            auto destroy_data = serializer.deserialize_entity_destroy(packet);
            renderer_.remove_entity(destroy_data.entity_id);
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing EntityDestroy packet: " << e.what() << std::endl;
        }
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

void Client::send_move(int8_t dx, int8_t dy) {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::PlayerMoveData move_data;
    move_data.player_id = player_id_;
    sf::Vector2f pos = renderer_.get_player_position(player_id_);
    move_data.position_x = pos.x;
    move_data.position_y = pos.y;
    move_data.velocity_x = dx;
    move_data.velocity_y = dy;
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
    sf::Vector2f pos = renderer_.get_player_position(player_id_);
    shoot_data.weapon_type = 0;
    shoot_data.position_x = pos.x;
    shoot_data.position_y = pos.y;
    shoot_data.direction_x = 1.0f;
    shoot_data.direction_y = 0.0f;
    rtype::net::Packet shoot_packet = serializer.serialize_player_shoot(shoot_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(shoot_packet);
    udp_client_->send(packet_data);
}

void Client::update() {
    network_system_.update(registry_);
}

} // namespace rtype::client
