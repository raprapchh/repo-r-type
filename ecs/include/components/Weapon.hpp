#pragma once
#include <string>

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
};

} // namespace rtype::ecs::component
