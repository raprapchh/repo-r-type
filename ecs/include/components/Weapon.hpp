#pragma once
#include <string>
#include "MovementPattern.hpp"

namespace rtype::ecs::component {

struct Weapon {
    bool isShooting = false;
    bool autoFire = false;
    float timeSinceLastFire = 0.0f;
    float fireRate = 0.5f;
    float damage = 10.0f;
    float projectileLifetime = 5.0f;
    float projectileSpeed = 1000.0f;
    float directionX = 1.0f;
    float directionY = 0.0f;
    float spawnOffsetX = 20.0f;
    float spawnOffsetY = 0.0f;
    std::string projectileTag = "BasicProjectile";
    int chargeLevel = 0;
    MovementPatternType projectilePattern = MovementPatternType::None;
    float projectileAmplitude = 0.0f;
    float projectileFrequency = 0.0f;
    float projectileWidth = 0.0f;
    float projectileHeight = 0.0f;
    bool ignoreScroll = false;
};

} // namespace rtype::ecs::component
