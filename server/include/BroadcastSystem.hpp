#pragma once

#include "ClientInfo.hpp"
#include "UdpServer.hpp"
#include "../../ecs/include/Registry.hpp"
#include "../../shared/interfaces/network/IProtocolAdapter.hpp"
#include "../../shared/interfaces/network/IMessageSerializer.hpp"
#include "../../shared/net/Packet.hpp"
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>

namespace rtype::server {

class BroadcastSystem {
  public:
    BroadcastSystem(GameEngine::Registry& registry, UdpServer& udp_server,
                    rtype::net::IProtocolAdapter& protocol_adapter, rtype::net::IMessageSerializer& message_serializer);

    void update(double dt, const std::map<std::string, ClientInfo>& clients);
    void send_initial_state(const std::string& ip, uint16_t port);

  private:
    void broadcast_spawns(const std::map<std::string, ClientInfo>& clients);
    void broadcast_moves(const std::map<std::string, ClientInfo>& clients);
    void broadcast_game_state(const std::map<std::string, ClientInfo>& clients, double elapsed_time);
    void broadcast_deaths(const std::map<std::string, ClientInfo>& clients);

    void broadcast_packet(const std::vector<uint8_t>& data, const std::map<std::string, ClientInfo>& clients);
    void send_to_client(const std::vector<uint8_t>& data, const std::string& ip, uint16_t port);

    GameEngine::Registry& registry_;
    UdpServer& udp_server_;
    rtype::net::IProtocolAdapter& protocol_adapter_;
    rtype::net::IMessageSerializer& message_serializer_;

    std::unordered_set<uint32_t> last_known_entities_;
    uint32_t next_network_id_ = 10000;
};

} // namespace rtype::server
