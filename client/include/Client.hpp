#pragma once

#include <asio.hpp>
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "../shared/net/Protocol.hpp"
#include "UdpClient.hpp"
#include "Renderer.hpp"

namespace rtype::client {

class Client {
  public:
    Client(const std::string& host, uint16_t port, Renderer& renderer);
    ~Client();

    void connect();
    void disconnect();
    void run();
    void send_move(int8_t dx, int8_t dy);
    void send_shoot(int32_t x, int32_t y);

  private:
    void handle_server_message(const std::vector<uint8_t>& data);
    void handle_udp_receive(const asio::error_code& error, std::size_t bytes_transferred,
                            const std::vector<uint8_t>& data);

    std::string host_;
    uint16_t port_;
    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<UdpClient> udp_client_;
    std::atomic<bool> connected_;
    uint32_t player_id_;
    Renderer& renderer_;
    std::unique_ptr<std::thread> network_thread_;
};

} // namespace rtype::client
