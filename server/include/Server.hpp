#pragma once

#include "UdpServer.hpp"
#include "GameSession.hpp"
#include "../../shared/interfaces/network/IProtocolAdapter.hpp"
#include "../../shared/interfaces/network/IMessageSerializer.hpp"
#include "../../shared/net/Packet.hpp"
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>

namespace rtype::server {

class Server {
  public:
    Server(uint16_t port = 4242);
    ~Server();

    void start();
    void stop();
    void run();

  private:
    void handle_client_message(const std::string& client_ip, uint16_t client_port, const std::vector<uint8_t>& data);
    void network_loop();

    GameSession* get_or_create_session(uint32_t session_id);
    uint32_t allocate_session_id();
    void unmap_client(const std::string& client_key);
    void remove_session(uint32_t session_id);

    uint16_t port_;
    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<UdpServer> udp_server_;
    std::unique_ptr<rtype::net::IProtocolAdapter> protocol_adapter_;
    std::unique_ptr<rtype::net::IMessageSerializer> message_serializer_;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> work_guard_;
    std::thread network_thread_;
    std::atomic<bool> running_;

    std::mutex sessions_mutex_;
    std::unordered_map<uint32_t, std::unique_ptr<GameSession>> sessions_;
    std::unordered_map<std::string, uint32_t> client_session_map_;
    std::unordered_map<uint32_t, std::string> session_names_;
    uint32_t next_session_id_;
};

} // namespace rtype::server
