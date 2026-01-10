#pragma once

#include <asio.hpp>
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <mutex>

namespace rtype::server {

/// @brief Asynchronous UDP server handling multiple clients
class UdpServer {
  public:
    using message_callback = std::function<void(const std::string&, uint16_t, const std::vector<uint8_t>&)>;

    /// @brief Constructs and binds server to specified port
    UdpServer(asio::io_context& io_ctx, uint16_t port);
    ~UdpServer();

    /// @brief Starts listening for incoming messages
    void start();

    /// @brief Stops the server
    void stop();

    /// @brief Sends data to a specific client
    void send(const std::string& client_ip, uint16_t client_port, const std::vector<uint8_t>& data);

    /// @brief Sets callback for received messages
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
    mutable std::mutex send_mutex_;
};

} // namespace rtype::server
