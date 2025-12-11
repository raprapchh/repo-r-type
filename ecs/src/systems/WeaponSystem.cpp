#include "../../include/systems/WeaponSystem.hpp"
#include "../../include/components/Weapon.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/Projectile.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/Tag.hpp"
#include "../../include/components/CollisionLayer.hpp"
#include "../../include/components/ScreenMode.hpp"
#include "../../shared/utils/GameConfig.hpp"
#include <iostream>
#include "../../../shared/utils/Logger.hpp"

namespace rtype::ecs {

void WeaponSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Weapon, component::Position>();

    view.each([&registry, dt](auto entity, component::Weapon& weapon, component::Position& pos) {
        weapon.timeSinceLastFire += static_cast<float>(dt);

        if (weapon.isShooting) {
        }

        if ((weapon.isShooting || weapon.autoFire) && weapon.timeSinceLastFire >= weapon.fireRate) {
            auto projectile = registry.createEntity();

            float spawnX = pos.x + weapon.spawnOffsetX;
            float spawnY = pos.y + weapon.spawnOffsetY;

            registry.addComponent<component::Position>(projectile, spawnX, spawnY);
            float vx = weapon.projectileSpeed * weapon.directionX;
            float vy = weapon.projectileSpeed * weapon.directionY;

            bool is_player = false;
            if (registry.hasComponent<component::Tag>(static_cast<std::size_t>(entity))) {
                const auto& tag = registry.getComponent<component::Tag>(static_cast<std::size_t>(entity));
                if (tag.name == "Player") {
                    is_player = true;
                }
            }

            if (!is_player) {
                vx -= rtype::config::SCROLL_SPEED;
            }
            float damage = weapon.damage;
            std::string projectileTag = weapon.projectileTag;
            float hitBoxW = 0.0f;
            float hitBoxH = 0.0f;

            if (!is_player) {
                hitBoxW = 110.0f;
                hitBoxH = 110.0f;
            } else {
                hitBoxW = 87.0f;
                hitBoxH = 99.0f;
            }

            registry.addComponent<component::Velocity>(projectile, vx, vy);

            if (registry.hasComponent<component::Tag>(static_cast<std::size_t>(entity))) {
                const auto& tag = registry.getComponent<component::Tag>(static_cast<std::size_t>(entity));
                if (tag.name == "Player") {
                    switch (weapon.chargeLevel) {
                    case 0:
                        projectileTag = "shot";
                        hitBoxW = 87.0f;
                        hitBoxH = 99.0f;
                        damage = 10.0f;
                        break;
                    case 1:
                        projectileTag = "shot_death-charge2";
                        hitBoxW = 120.0f;
                        hitBoxH = 120.0f;
                        damage = 20.0f;
                        break;
                    case 2:
                        projectileTag = "shot_death-charge3";
                        hitBoxW = 150.0f;
                        hitBoxH = 150.0f;
                        damage = 30.0f;
                        break;
                    case 3:
                        projectileTag = "shot_death-charge4";
                        hitBoxW = 180.0f;
                        hitBoxH = 180.0f;
                        damage = 40.0f;
                        break;
                    }
                }
            }

            auto& projComp =
                registry.addComponent<component::Projectile>(projectile, damage, weapon.projectileLifetime);
            projComp.owner_id = static_cast<std::size_t>(entity);

            registry.addComponent<component::HitBox>(projectile, hitBoxW, hitBoxH);
            registry.addComponent<component::Tag>(projectile, projectileTag);

            component::CollisionLayer projLayer = component::CollisionLayer::PlayerProjectile;
            if (registry.hasComponent<component::Collidable>(static_cast<std::size_t>(entity))) {
                auto& ownerCollidable = registry.getComponent<component::Collidable>(static_cast<std::size_t>(entity));
                if (ownerCollidable.layer == component::CollisionLayer::Enemy) {
                    projLayer = component::CollisionLayer::EnemyProjectile;
                }
            }
            registry.addComponent<component::Collidable>(projectile, projLayer);

            weapon.timeSinceLastFire = 0.0f;
            if (!weapon.autoFire) {
                weapon.isShooting = false;
            }
        }
    });
}

} // namespace rtype::ecs
