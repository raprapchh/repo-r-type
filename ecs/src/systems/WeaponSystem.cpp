#include "../../include/systems/WeaponSystem.hpp"
#include "../../include/components/Weapon.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/Projectile.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/Tag.hpp"
#include "../../include/components/CollisionLayer.hpp"
#include <iostream>

namespace rtype::ecs {

void WeaponSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Weapon, component::Position>();

    view.each([&registry, dt](auto entity, component::Weapon& weapon, component::Position& pos) {
        weapon.timeSinceLastFire += static_cast<float>(dt);

        if ((weapon.isShooting || weapon.autoFire) && weapon.timeSinceLastFire >= weapon.fireRate) {
            auto projectile = registry.createEntity();

            float spawnX = pos.x + weapon.spawnOffsetX;
            float spawnY = pos.y + weapon.spawnOffsetY;

            registry.addComponent<component::Position>(projectile, spawnX, spawnY);
            registry.addComponent<component::Velocity>(projectile, weapon.projectileSpeed * weapon.directionX,
                                                       weapon.projectileSpeed * weapon.directionY);

            auto& projComp =
                registry.addComponent<component::Projectile>(projectile, weapon.damage, weapon.projectileLifetime);
            projComp.owner_id = static_cast<std::size_t>(entity);

            registry.addComponent<component::HitBox>(projectile, 58.0f, 66.0f);
            registry.addComponent<component::Tag>(projectile, weapon.projectileTag);

            component::CollisionLayer layer = component::CollisionLayer::None;
            if (registry.hasComponent<component::Tag>(static_cast<std::size_t>(entity))) {
                const auto& tag = registry.getComponent<component::Tag>(static_cast<std::size_t>(entity));
                if (tag.name == "Player") {
                    layer = component::CollisionLayer::PlayerProjectile;
                } else if (tag.name.find("Monster") != std::string::npos) {
                    layer = component::CollisionLayer::EnemyProjectile;
                }
            }
            registry.addComponent<component::Collidable>(projectile, layer);

            weapon.timeSinceLastFire = 0.0f;
            if (!weapon.autoFire) {
                weapon.isShooting = false;
            }
        }
    });
}

} // namespace rtype::ecs
