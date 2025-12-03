#pragma once

#include <queue>
#include <memory>
#include "../../shared/net/Packet.hpp"
#include "../../shared/net/MessageSerializer.hpp"
#include "../../ecs/include/Registry.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Velocity.hpp"
#include "../../ecs/include/components/NetworkId.hpp"

namespace rtype::client {

class NetworkSystem {
  public:
    NetworkSystem();
    ~NetworkSystem() = default;

    void update(GameEngine::Registry& registry);
    void push_packet(const rtype::net::Packet& packet);

  private:
    void handle_spawn(GameEngine::Registry& registry, const rtype::net::Packet& packet);
    void handle_move(GameEngine::Registry& registry, const rtype::net::Packet& packet);
    void handle_destroy(GameEngine::Registry& registry, const rtype::net::Packet& packet);

    std::queue<rtype::net::Packet> packet_queue_;
    rtype::net::MessageSerializer serializer_;
};

} // namespace rtype::client
