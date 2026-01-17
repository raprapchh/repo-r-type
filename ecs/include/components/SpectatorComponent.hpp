#pragma once

#include <cstddef>

namespace rtype::ecs::component {

/// @brief Component to mark an entity as a spectator
/// Attached to the local client entity when in spectator mode
struct SpectatorComponent {
    std::size_t targetEntityId = 0; // ID of the player we are watching
    float switchTimer = 0.0f;       // Cooldown for switching targets

    SpectatorComponent() = default;

    explicit SpectatorComponent(std::size_t target) : targetEntityId(target), switchTimer(0.0f) {
    }
};

} // namespace rtype::ecs::component
