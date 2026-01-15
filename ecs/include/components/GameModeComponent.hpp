#pragma once

namespace rtype::ecs::component {

/// @brief Component to store game mode information
/// Attached to a global/world entity for systems to access
struct GameModeComponent {
    bool is_multiplayer = false;

    GameModeComponent() = default;

    explicit GameModeComponent(bool multiplayer) : is_multiplayer(multiplayer) {
    }
};

} // namespace rtype::ecs::component
