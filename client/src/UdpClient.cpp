#include "UdpClient.hpp"
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace rtype::client {

UdpClient::UdpClient(asio::io_context& io_context, const std::string& host, uint16_t port)
    : io_context_(io_context), running_(false) {
    try {
        socket_ = std::make_unique<asio::ip::udp::socket>(io_context_);

        asio::ip::udp::resolver resolver(io_context_);
        server_endpoint_ = *resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port)).begin();
        
        socket_->open(asio::ip::udp::v4());
        // Bind the socket to a local endpoint. Port 0 means the OS will choose a port.
        socket_->bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));

        recv_buffer_.resize(1024);
        std::cout << "UdpClient initialized and bound to local port: " << socket_->local_endpoint().port() << std::endl;
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
    socket_->async_receive_from(
        asio::buffer(recv_buffer_), remote_endpoint_,
        [this](const asio::error_code& error, std::size_t bytes_transferred) {
            handle_receive(error, bytes_transferred);
        }
    );
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
    if (!running_) {
        std::cout << "handle_receive called but client is not running." << std::endl;
        return;
    }

    if (error) {
#ifdef _WIN32
        if (error.value() == WSAECONNRESET) {
            std::cout << "[DEBUG] WSAECONNRESET received, ignoring and continuing." << std::endl;
            start_receive();
            return;
        }
#endif
        if (error == asio::error::operation_aborted) {
            std::cout << "[DEBUG] Operation aborted." << std::endl;
            return;
        }

        std::cerr << "UdpClient receive error: " << error.message() << " (code: " << error.value() << ")" << std::endl;
        if (handler_) {
            handler_(error, bytes_transferred, {});
        }
        start_receive();
        return;
    }

    if (bytes_transferred > 0) {
        std::vector<uint8_t> received_data(recv_buffer_.begin(), recv_buffer_.begin() + bytes_transferred);
        if (handler_) {
            handler_(error, bytes_transferred, received_data);
        }
    } else {
        std::cerr << "[WARNING] Received empty packet, ignoring." << std::endl;
    }

    start_receive();
}

} // namespace rtype::client