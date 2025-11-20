#pragma once

#include <asio.hpp>
#include <memory>
#include <functional>
#include <array>
#include <map>

namespace rtype::server {

    class UdpServer {
    public:
        using message_callback = std::function<void(const std::string&, uint16_t, const std::vector<uint8_t>&)>;

        UdpServer(asio::io_context& io_ctx, uint16_t port);
        ~UdpServer();

        void start();
        void stop();
        void send(const std::string& client_ip, uint16_t client_port, const std::vector<uint8_t>& data);
        void set_message_handler(message_callback handler);

    private:
        void start_receive();
        void handle_receive(const asio::error_code& error, size_t bytes_transferred);

        asio::io_context& io_context_;
        std::unique_ptr<asio::ip::udp::socket> socket_;
        asio::ip::udp::endpoint remote_endpoint_;
        std::array<uint8_t, 1024> recv_buffer_;
        message_callback handler_;
        bool running_;
    };
}
