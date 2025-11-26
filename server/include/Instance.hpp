#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "../../ecs/include/Registry.hpp"

namespace rtype::server {

struct RoomPlayer {
    uint32_t player_id;
    std::string ip;
    uint16_t port;
    GameEngine::entity_t entity;
    bool is_connected{true};
};

class Instance {
  public:
    explicit Instance(std::string name = "default_room");
    ~Instance();
    const std::string& name() const noexcept;
    GameEngine::Registry& registry() noexcept;

  private:
    std::string name_;
    GameEngine::Registry registry_;
};

} // namespace rtype::server
