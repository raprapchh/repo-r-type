#pragma once

#include <asio.hpp>
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "../shared/net/Protocol.hpp"

namespace rtype::client {

class Client {
public:
    Client(const std::string& host, uint16_t port);
    ~Client();

    void connect();
    void disconnect();
    void run();

private:
    void handle_server_message(const std::vector<uint8_t>& data);

    std::string host_;
    uint16_t port_;
    std::unique_ptr<asio::io_context> io_context_;
    std::atomic<bool> connected_;
    uint32_t player_id_;
    std::unique_ptr<std::thread> network_thread_;
};

} // namespace rtype::client
