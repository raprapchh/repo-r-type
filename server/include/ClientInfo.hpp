#pragma once

#include "../../ecs/include/Registry.hpp"
#include <chrono>
#include <string>

namespace rtype::server {

struct ClientInfo {
    std::string ip;
    uint16_t port;
    uint32_t player_id;
    std::string player_name;
    bool is_connected;
    GameEngine::entity_t entity_id;
    std::chrono::steady_clock::time_point last_seen;
};

} // namespace rtype::server
