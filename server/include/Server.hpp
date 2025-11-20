#pragma once

#include "../shared/net/Packet.hpp"
#include "../shared/net/Protocol.hpp"
#include "UdpServer.hpp"
#include <map>
#include <memory>
#include <string>

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
    void handle_client_message(const std::string& client_ip, uint16_t client_port,
                               const std::vector<uint8_t>& data);
    void handle_player_join(const std::string& client_ip, uint16_t client_port);
    void handle_player_move(const std::string& client_ip, uint16_t client_port,
                            const std::vector<uint8_t>& data);
    void broadcast_message(const std::vector<uint8_t>& data, const std::string& exclude_ip = "",
                           uint16_t exclude_port = 0);

    uint16_t port_;
    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<UdpServer> udp_server_;
    std::map<std::string, ClientInfo> clients_;
    uint32_t next_player_id_;
    bool running_;
};

} // namespace rtype::server
