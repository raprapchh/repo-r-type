#include "UdpServer.hpp"
#include <iostream>

namespace rtype::server {

UdpServer::UdpServer(asio::io_context& io_ctx, uint16_t port) : io_context_(io_ctx), running_(false) {
    try {
        socket_ =
            std::make_unique<asio::ip::udp::socket>(io_context_, asio::ip::udp::endpoint(asio::ip::udp::v4(), port));
    } catch (const std::exception& e) {
        std::cerr << "Failed to create UDP socket: " << e.what() << std::endl;
    }
}

UdpServer::~UdpServer() {
    stop();
}

void UdpServer::start() {
    if (!socket_ || socket_->is_open() == false) {
        return;
    }
    running_ = true;
    start_receive();
}

void UdpServer::stop() {
    running_ = false;
    if (socket_ && socket_->is_open()) {
        socket_->close();
    }
}

void UdpServer::send(const std::string& client_ip, uint16_t client_port, const std::vector<uint8_t>& data) {
    if (!socket_ || !running_) {
        return;
    }

    try {
        asio::ip::udp::endpoint endpoint(asio::ip::make_address(client_ip), client_port);
        socket_->send_to(asio::buffer(data), endpoint);
    } catch (const std::exception& e) {
        std::cerr << "Send error: " << e.what() << std::endl;
    }
}

void UdpServer::set_message_handler(message_callback handler) {
    handler_ = handler;
}

void UdpServer::start_receive() {
    if (!socket_ || !running_) {
        return;
    }

    socket_->async_receive_from(
        asio::buffer(recv_buffer_), remote_endpoint_,
        [this](const asio::error_code& error, size_t bytes_transferred) { handle_receive(error, bytes_transferred); });
}

void UdpServer::handle_receive(const asio::error_code& error, size_t bytes_transferred) {
    if (!error && bytes_transferred > 0) {
        std::vector<uint8_t> buffer(recv_buffer_.begin(), recv_buffer_.begin() + bytes_transferred);

        if (handler_) {
            handler_(remote_endpoint_.address().to_string(), remote_endpoint_.port(), buffer);
        }
    }

    if (running_) {
        start_receive();
    }
}

} // namespace rtype::server
