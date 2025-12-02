#pragma once

#include <asio.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace rtype::client {

class UdpClient {
  public:
    using message_callback = std::function<void(const asio::error_code&, std::size_t, const std::vector<uint8_t>&)>;

    UdpClient(asio::io_context& io_context, const std::string& host, uint16_t port);
    ~UdpClient();

    void start_receive();
    void send(const std::vector<uint8_t>& data);
    void stop();

    void set_message_handler(message_callback handler);

  private:
    void handle_receive(const asio::error_code& error, std::size_t bytes_transferred);

    asio::io_context& io_context_;
    std::unique_ptr<asio::ip::udp::socket> socket_;
    asio::ip::udp::endpoint server_endpoint_;
    std::vector<uint8_t> recv_buffer_;
    message_callback handler_;
    std::atomic<bool> running_;
};

} // namespace rtype::client