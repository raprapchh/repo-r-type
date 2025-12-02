#include "Client.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include <iostream>

namespace rtype::client {

Client::Client(const std::string& host, uint16_t port)
    : host_(host), port_(port), connected_(false), player_id_(0) {
    io_context_ = std::make_unique<asio::io_context>();
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

    std::cout << "Connection request sent to " << host_ << ":" << port_ << std::endl;

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
    std::cout << "Disconnected from server." << std::endl;
}

void Client::run() {
    network_thread_ = std::make_unique<std::thread>([this]() {
        io_context_->run();
    });
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

} // namespace rtype::client
