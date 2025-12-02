#include "Client.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include <iostream>

namespace rtype::client {

Client::Client(const std::string& host, uint16_t port) : host_(host), port_(port), connected_(false), player_id_(0) {
    io_context_ = std::make_unique<asio::io_context>();
    udp_client_ = std::make_unique<UdpClient>(*io_context_, host_, port_);
    udp_client_->set_message_handler(
        [this](const asio::error_code& error, std::size_t bytes_transferred, const std::vector<uint8_t>& data) {
            handle_udp_receive(error, bytes_transferred, data);
        });
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
    if (!connected_.load()) {
        return;
    }
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
    std::cout << "Disconnected from server." << std::endl;
}

void Client::run() {
    network_thread_ = std::make_unique<std::thread>([this]() { io_context_->run(); });
}

void Client::handle_udp_receive(const asio::error_code& error, std::size_t bytes_transferred,
                                const std::vector<uint8_t>& data) {
    if (!error) {
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
        std::cerr << "Invalid packet received." << std::endl;
        return;
    }
    rtype::net::Packet packet = adapter.deserialize(data);
    rtype::net::MessageSerializer serializer;

    switch (static_cast<rtype::net::MessageType>(packet.header.message_type)) {
    case rtype::net::MessageType::PlayerJoin: {
        auto join_data = serializer.deserialize_player_join(packet);
        if (!connected_.load()) {
            player_id_ = join_data.player_id;
            connected_ = true;
            std::cout << "Successfully connected to server. My Player ID is " << player_id_ << std::endl;
        } else {
            std::cout << "Player " << join_data.player_id << " has joined the game." << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::GameState:
        std::cout << "Received game state update." << std::endl;
        // Here you would deserialize and process the game state
        break;
    case rtype::net::MessageType::PlayerMove:
        std::cout << "Received player move update." << std::endl;
        // Here you would deserialize and process the move data
        break;
    case rtype::net::MessageType::Pong: {
        auto pong_data = serializer.deserialize_ping_pong(packet);
        // Here you can calculate latency
        std::cout << "Received Pong." << std::endl;
        break;
    }
    default:
        std::cout << "Unknown message type received: " << packet.header.message_type << std::endl;
        break;
    }
}

void Client::send_move(int8_t dx, int8_t dy) {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::PlayerMoveData move_data;
    move_data.player_id = player_id_;
    move_data.dx = dx;
    move_data.dy = dy;
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
    shoot_data.x = x;
    shoot_data.y = y;
    rtype::net::Packet shoot_packet = serializer.serialize_player_shoot(shoot_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(shoot_packet);
    udp_client_->send(packet_data);
}

} // namespace rtype::client
