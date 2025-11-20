#include "Server.hpp"
#include "../shared/net/Deserializer.hpp"
#include "../shared/net/Serializer.hpp"
#include <iostream>

namespace rtype::server {

Server::Server(uint16_t port) : port_(port), next_player_id_(1), running_(false) {
    io_context_ = std::make_unique<asio::io_context>();
    udp_server_ = std::make_unique<UdpServer>(*io_context_, port_);
}

Server::~Server() {
    stop();
}

void Server::start() {
    if (running_) {
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
    running_ = false;
    if (udp_server_) {
        udp_server_->stop();
    }
}

void Server::run() {
    start();
    if (io_context_) {
        io_context_->run();
    }
}

void Server::handle_client_message(const std::string& client_ip, uint16_t client_port,
                                   const std::vector<uint8_t>& data) {
    rtype::net::Packet packet = rtype::net::Packet::deserialize(data);

    std::string client_key = client_ip + ":" + std::to_string(client_port);

    switch (static_cast<rtype::net::MessageType>(packet.header.message_type)) {
    case rtype::net::MessageType::PlayerJoin:
        handle_player_join(client_ip, client_port);
        break;

    case rtype::net::MessageType::PlayerMove:
        handle_player_move(client_ip, client_port, packet.body);
        break;

    case rtype::net::MessageType::Ping: {
        rtype::net::Serializer serializer;
        serializer.write(static_cast<uint16_t>(rtype::net::MessageType::Pong));
        rtype::net::Packet pong_packet(static_cast<uint16_t>(rtype::net::MessageType::Pong), serializer.get_data());
        udp_server_->send(client_ip, client_port, pong_packet.serialize());
    } break;

    default:
        std::cout << "Unknown message type: " << packet.header.message_type << std::endl;
    }
}

void Server::handle_player_join(const std::string& client_ip, uint16_t client_port) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    if (clients_.find(client_key) != clients_.end()) {
        return;
    }

    ClientInfo info;
    info.ip = client_ip;
    info.port = client_port;
    info.player_id = next_player_id_++;
    info.is_connected = true;

    clients_[client_key] = info;

    std::cout << "Player " << info.player_id << " joined from " << client_ip << ":" << client_port << std::endl;

    rtype::net::Serializer serializer;
    serializer.write(info.player_id);

    rtype::net::Packet response(static_cast<uint16_t>(rtype::net::MessageType::PlayerJoin), serializer.get_data());

    udp_server_->send(client_ip, client_port, response.serialize());
    broadcast_message(response.serialize(), client_ip, client_port);
}

void Server::handle_player_move(const std::string& client_ip, uint16_t client_port, const std::vector<uint8_t>& data) {
    std::string client_key = client_ip + ":" + std::to_string(client_port);

    auto it = clients_.find(client_key);
    if (it == clients_.end()) {
        return;
    }

    broadcast_message(data, client_ip, client_port);
}

void Server::broadcast_message(const std::vector<uint8_t>& data, const std::string& exclude_ip, uint16_t exclude_port) {
    for (const auto& [key, client] : clients_) {
        if (client.is_connected && (client.ip != exclude_ip || client.port != exclude_port)) {
            udp_server_->send(client.ip, client.port, data);
        }
    }
}

} // namespace rtype::server
