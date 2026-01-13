#pragma once

namespace rtype::ecs::component {

/// @brief Power-up type enumeration
enum class PowerUpTypeEnum { HEALTH = 0, FORCE_POD = 1 };

/// @brief Component to identify power-up type
struct PowerUpType {
    PowerUpTypeEnum type = PowerUpTypeEnum::HEALTH;

    PowerUpType() = default;
    PowerUpType(PowerUpTypeEnum t) : type(t) {
    }
};

} // namespace rtype::ecs::component
