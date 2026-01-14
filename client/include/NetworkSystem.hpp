#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include "net/Packet.hpp"
#include "net/MessageSerializer.hpp"
#include "Registry.hpp"
#include "components/Position.hpp"
#include "components/Velocity.hpp"
#include "components/NetworkId.hpp"
#include "components/Drawable.hpp"
#include "components/Controllable.hpp"

namespace rtype::client {

class NetworkSystem {
  public:
    NetworkSystem(uint32_t player_id = 0);
    ~NetworkSystem() = default;

    void update(GameEngine::Registry& registry, std::mutex& registry_mutex);
    void push_packet(const rtype::net::Packet& packet);
    void set_player_id(uint32_t player_id);

  private:
    void handle_spawn(GameEngine::Registry& registry, const rtype::net::Packet& packet);
    void handle_move(GameEngine::Registry& registry, const rtype::net::Packet& packet);
    void handle_destroy(GameEngine::Registry& registry, const rtype::net::Packet& packet);
    void handle_pong(GameEngine::Registry& registry, const rtype::net::Packet& packet);

    std::queue<rtype::net::Packet> packet_queue_;
    std::mutex packet_queue_mutex_;
    rtype::net::MessageSerializer serializer_;
    uint32_t player_id_;
};

} // namespace rtype::client
