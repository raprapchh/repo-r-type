#pragma once

#include <string>

namespace rtype::ecs::component {

/// @brief Component storing the player's display name
struct PlayerName {
    std::string name;

    PlayerName(const std::string& n = "Player") : name(n) {
    }
};

} // namespace rtype::ecs::component
