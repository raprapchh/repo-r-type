#include "../../include/systems/WeaponSystem.hpp"
#include "../../include/components/MovementPattern.hpp"
#include "../../include/components/Weapon.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/Projectile.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/Tag.hpp"
#include "../../include/components/CollisionLayer.hpp"
#include "../../include/components/ScreenMode.hpp"
#include "../../include/components/AudioEvent.hpp"
#include "../../shared/utils/GameConfig.hpp"
#include <vector>
#include "../../../shared/utils/Logger.hpp"

namespace rtype::ecs {

void WeaponSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Weapon, component::Position>();

    struct ProjectileRequest {
        float x, y;
        float vx, vy;
        float damage;
        float lifetime;
        std::string tag;
        float w, h;
        component::CollisionLayer layer;
        std::size_t ownerId;
        component::MovementPatternType patternType;
        float patternAmplitude;
        float patternFrequency;
    };

    std::vector<ProjectileRequest> requests;

    view.each([&](auto entity, component::Weapon& weapon, component::Position& pos) {
        weapon.timeSinceLastFire += static_cast<float>(dt);

        if (weapon.isShooting) {
        }

        if (registry.hasComponent<component::Tag>(static_cast<std::size_t>(entity))) {
            const auto& tag = registry.getComponent<component::Tag>(static_cast<std::size_t>(entity));

            if (tag.name == "Monster_Wave_2_Left" || tag.name == "Monster_Wave_2_Right") {
                if (registry.hasComponent<component::Velocity>(static_cast<std::size_t>(entity))) {
                    const auto& vel = registry.getComponent<component::Velocity>(static_cast<std::size_t>(entity));
                    if (std::abs(vel.vy) < 20.0f && weapon.timeSinceLastFire >= 1.0f) {
                        float spawnX = pos.x + weapon.spawnOffsetX;
                        float spawnY = pos.y + weapon.spawnOffsetY;
                        float baseVx = (tag.name == "Monster_Wave_2_Left") ? 400.0f : -400.0f;

                        requests.push_back({spawnX, spawnY, baseVx, 0.0f, 10.0f, 3.0f, "PodProjectileRed", 36.0f, 13.0f,
                                            component::CollisionLayer::EnemyProjectile,
                                            static_cast<std::size_t>(entity), component::MovementPatternType::None,
                                            0.0f, 0.0f});

                        requests.push_back({spawnX, spawnY, baseVx * 0.9f, -150.0f, 10.0f, 3.0f, "PodProjectileRed",
                                            36.0f, 13.0f, component::CollisionLayer::EnemyProjectile,
                                            static_cast<std::size_t>(entity), component::MovementPatternType::None,
                                            0.0f, 0.0f});

                        requests.push_back({spawnX, spawnY, baseVx * 0.9f, 150.0f, 10.0f, 3.0f, "PodProjectileRed",
                                            36.0f, 13.0f, component::CollisionLayer::EnemyProjectile,
                                            static_cast<std::size_t>(entity), component::MovementPatternType::None,
                                            0.0f, 0.0f});

                        weapon.timeSinceLastFire = 0.0f;
                    }
                }
                return;
            }
        }

        if ((weapon.isShooting || weapon.autoFire) && weapon.timeSinceLastFire >= weapon.fireRate) {
            float spawnX = pos.x + weapon.spawnOffsetX;
            float spawnY = pos.y + weapon.spawnOffsetY;

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
                hitBoxW = 70.0f;
                hitBoxH = 70.0f;
            } else {
                hitBoxW = 87.0f;
                hitBoxH = 99.0f;
            }

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
                        hitBoxW = 80.0f;
                        hitBoxH = 80.0f;
                        damage = 20.0f;
                        break;
                    case 2:
                        projectileTag = "shot_death-charge3";
                        hitBoxW = 100.0f;
                        hitBoxH = 100.0f;
                        damage = 30.0f;
                        break;
                    case 3:
                        projectileTag = "shot_death-charge4";
                        hitBoxW = 120.0f;
                        hitBoxH = 120.0f;
                        damage = 40.0f;
                        break;
                    }
                }
            }

            component::CollisionLayer projLayer = component::CollisionLayer::PlayerProjectile;
            if (registry.hasComponent<component::Collidable>(static_cast<std::size_t>(entity))) {
                auto& ownerCollidable = registry.getComponent<component::Collidable>(static_cast<std::size_t>(entity));
                if (ownerCollidable.layer == component::CollisionLayer::Enemy) {
                    projLayer = component::CollisionLayer::EnemyProjectile;
                } else if (ownerCollidable.layer == component::CollisionLayer::Companion) {
                    projLayer = component::CollisionLayer::PlayerProjectile;
                }
            }

            float freq = weapon.projectileFrequency;
            if (projectileTag == "Boss_1_Bayblade") {
                if (rand() % 2 == 0) {
                    freq = -freq;
                }
            }

            requests.push_back({spawnX, spawnY, vx, vy, damage, weapon.projectileLifetime, projectileTag, hitBoxW,
                                hitBoxH, projLayer, static_cast<std::size_t>(entity), weapon.projectilePattern,
                                weapon.projectileAmplitude, freq});

            weapon.timeSinceLastFire = 0.0f;
            if (!weapon.autoFire) {
                weapon.isShooting = false;
            }
        }
    });

    for (const auto& req : requests) {
        auto projectile = registry.createEntity();
        registry.addComponent<component::Position>(projectile, req.x, req.y);
        registry.addComponent<component::Velocity>(projectile, req.vx, req.vy);
        auto& projComp = registry.addComponent<component::Projectile>(projectile, req.damage, req.lifetime);
        projComp.owner_id = req.ownerId;
        registry.addComponent<component::HitBox>(projectile, req.w, req.h);
        registry.addComponent<component::Tag>(projectile, req.tag);
        registry.addComponent<component::Collidable>(projectile, req.layer);
        if (req.patternType != component::MovementPatternType::None) {
            registry.addComponent<component::MovementPattern>(projectile, req.patternType, 0.0f, req.patternAmplitude,
                                                              req.patternFrequency);
        }

        // Add audio event for shooting
        bool is_player =
            (req.layer == component::CollisionLayer::PlayerProjectile || req.tag.find("shot") != std::string::npos);
        if (is_player) {
            registry.addComponent<component::AudioEvent>(projectile, component::AudioEventType::PLAYER_SHOOT);
        } else {
            registry.addComponent<component::AudioEvent>(projectile, component::AudioEventType::ENEMY_SHOOT);
        }
    }
}

} // namespace rtype::ecs
