#include "UdpClient.hpp"
#include <iostream>

namespace rtype::client {

UdpClient::UdpClient(asio::io_context& io_context, const std::string& host, uint16_t port)
    : io_context_(io_context), running_(false) {
    try {
        socket_ = std::make_unique<asio::ip::udp::socket>(io_context_);

        asio::ip::udp::resolver resolver(io_context_);
        server_endpoint_ = *resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port)).begin();
        socket_->open(asio::ip::udp::v4());

        recv_buffer_.resize(1024);
    } catch (const std::exception& e) {
        std::cerr << "UdpClient initialization error: " << e.what() << std::endl;
    }
}

UdpClient::~UdpClient() {
    stop();
}

void UdpClient::start_receive() {
    if (!socket_ || !socket_->is_open()) {
        std::cerr << "UdpClient: Socket not open for receiving." << std::endl;
        return;
    }
    running_ = true;
    remote_endpoint_ = asio::ip::udp::endpoint();
    socket_->async_receive_from(asio::buffer(recv_buffer_), remote_endpoint_,
                                [this](const asio::error_code& error, std::size_t bytes_transferred) {
                                    handle_receive(error, bytes_transferred);
                                });
}

void UdpClient::send(const std::vector<uint8_t>& data) {
    if (!socket_ || !socket_->is_open()) {
        std::cerr << "UdpClient: Socket not open for sending." << std::endl;
        return;
    }
    try {
        socket_->send_to(asio::buffer(data), server_endpoint_);
    } catch (const std::exception& e) {
        std::cerr << "UdpClient send error: " << e.what() << std::endl;
    }
}

void UdpClient::stop() {
    running_ = false;
    if (socket_ && socket_->is_open()) {
        asio::error_code ec;
        socket_->close(ec);
        if (ec) {
            std::cerr << "UdpClient close error: " << ec.message() << std::endl;
        }
    }
}

void UdpClient::set_message_handler(message_callback handler) {
    handler_ = std::move(handler);
}

void UdpClient::handle_receive(const asio::error_code& error, std::size_t bytes_transferred) {
    std::cout << "[DEBUG] UdpClient initialized" << std::endl;
    std::cout << "[DEBUG] Starting async_receive_from" << std::endl;
    std::cout << "[DEBUG] Packet from: "
            << remote_endpoint_.address().to_string()
            << ":" << remote_endpoint_.port()
            << " (" << bytes_transferred << " bytes)" << std::endl;
    if (!error) {
        if (bytes_transferred == 0) {
            std::cerr << "[WARNING] Received empty packet, ignoring." << std::endl;
        } else {
            std::vector<uint8_t> received_data(recv_buffer_.begin(), recv_buffer_.begin() + bytes_transferred);
            if (handler_) {
                handler_(error, bytes_transferred, received_data);
            }
        }
        if (running_) {
            start_receive();
        }
    } else if (error != asio::error::operation_aborted) {
        std::cerr << "UdpClient receive error: " << error.message() << std::endl;
        if (handler_) {
            handler_(error, bytes_transferred, {});
        }
        if (running_) {
            start_receive();
        }
    }
}

} // namespace rtype::client
