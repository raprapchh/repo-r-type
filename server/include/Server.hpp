#pragma once

#include "UdpServer.hpp"
#include "../../ecs/include/Registry.hpp"
#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace rtype::server {

struct ClientInfo {
    std::string ip;
    uint16_t port;
    uint32_t player_id;
    bool is_connected;
};

class Server {
  public:
    Server(uint16_t port = 4242);
    ~Server();

    void start();
    void stop();
    void run();

  private:
    void handle_client_message(const std::string& client_ip, uint16_t client_port, const std::vector<uint8_t>& data);
    void handle_player_join(const std::string& client_ip, uint16_t client_port);
    void handle_player_move(const std::string& client_ip, uint16_t client_port, const std::vector<uint8_t>& data);
    void broadcast_message(const std::vector<uint8_t>& data, const std::string& exclude_ip = "",
                           uint16_t exclude_port = 0);
    void game_loop();
    void network_loop();

    uint16_t port_;
    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<UdpServer> udp_server_;
    std::map<std::string, ClientInfo> clients_;
    std::mutex clients_mutex_;
    uint32_t next_player_id_;
    std::atomic<bool> running_;
    std::thread game_thread_;
    std::thread network_thread_;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> work_guard_;
    static constexpr double TARGET_TICK_RATE = 60.0;
    static constexpr std::chrono::milliseconds TICK_DURATION =
        std::chrono::milliseconds(static_cast<long>(1000.0 / TARGET_TICK_RATE));

    GameEngine::Registry registry_;
};

} // namespace rtype::server
