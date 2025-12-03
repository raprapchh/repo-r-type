#pragma once

namespace rtype::ecs::component {

struct Weapon {
    bool isShooting = false;
    float timeSinceLastFire = 0.0f;
    float fireRate = 0.5f;
    float damage = 10.0f;
    float projectileLifetime = 2.0f;
    float projectileSpeed = 500.0f;
};

} // namespace rtype::ecs::component
