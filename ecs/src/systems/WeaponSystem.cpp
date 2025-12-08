#include "../../include/systems/WeaponSystem.hpp"
#include "../../include/components/Weapon.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/Projectile.hpp"
#include "../../include/components/HitBox.hpp"
#include <iostream>

namespace rtype::ecs {

void WeaponSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Weapon, component::Position>();

    view.each([&registry, dt](component::Weapon& weapon, component::Position& pos) {
        weapon.timeSinceLastFire += static_cast<float>(dt);

        if (weapon.isShooting && weapon.timeSinceLastFire >= weapon.fireRate) {
            auto projectile = registry.createEntity();

            float spawnX = pos.x + 20.0f;
            float spawnY = pos.y;

            registry.addComponent<component::Position>(projectile, spawnX, spawnY);
            registry.addComponent<component::Velocity>(projectile, weapon.projectileSpeed, 0.0f);
            registry.addComponent<component::Projectile>(projectile, weapon.damage, weapon.projectileLifetime);
            registry.addComponent<component::HitBox>(projectile, 58.0f, 66.0f);

            weapon.timeSinceLastFire = 0.0f;
            weapon.isShooting = false;
        }
    });
}

} // namespace rtype::ecs
