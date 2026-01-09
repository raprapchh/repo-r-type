#include "../../include/systems/CollisionSystem.hpp"
#include <vector>
#include <iostream>
#include <cstdlib>
#include "../../include/components/Position.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/CollisionLayer.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Projectile.hpp"
#include "../../include/components/Score.hpp"
#include "../../include/components/Lives.hpp"
#include "../../include/components/Tag.hpp"
#include "../../include/components/PowerUpType.hpp"
#include "../../include/components/Parent.hpp"
#include "../../include/components/SpawnEffect.hpp"
#include "../../include/components/Drawable.hpp"
#include "../../include/components/Weapon.hpp"
#include "../../include/components/NetworkId.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/HitFlash.hpp"
#include "../../include/components/StageCleared.hpp"

namespace rtype::ecs {

// Forward declarations for factory functions
void spawnForcePodItem(GameEngine::Registry& registry, float x, float y);
void spawnForcePodCompanion(GameEngine::Registry& registry, GameEngine::entity_t playerId);

void CollisionSystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;
    auto view = registry.view<component::Position, component::HitBox, component::Collidable>();

    std::vector<GameEngine::entity_t> entities;
    for (auto entity : view) {
        entities.push_back(static_cast<GameEngine::entity_t>(entity));
    }

    for (size_t i = 0; i < entities.size(); ++i) {
        auto entity1 = entities[i];
        if (!registry.isValid(entity1)) {
            continue;
        }

        if (!registry.hasComponent<component::Position>(entity1) ||
            !registry.hasComponent<component::HitBox>(entity1) ||
            !registry.hasComponent<component::Collidable>(entity1)) {
            continue;
        }

        auto& pos1 = view.get<component::Position>(entity1);
        auto& hitbox1 = view.get<component::HitBox>(entity1);
        auto& collidable1 = view.get<component::Collidable>(entity1);

        if (!collidable1.is_active) {
            continue;
        }

        for (size_t j = i + 1; j < entities.size(); ++j) {
            auto entity2 = entities[j];
            if (!registry.isValid(entity2)) {
                continue;
            }

            if (!registry.hasComponent<component::Position>(entity2) ||
                !registry.hasComponent<component::HitBox>(entity2) ||
                !registry.hasComponent<component::Collidable>(entity2)) {
                continue;
            }

            auto& pos2 = view.get<component::Position>(entity2);
            auto& hitbox2 = view.get<component::HitBox>(entity2);
            auto& collidable2 = view.get<component::Collidable>(entity2);

            if (!collidable2.is_active) {
                continue;
            }

            if (!ShouldCollide(collidable1.layer, collidable2.layer)) {
                continue;
            }

            if (CheckAABBCollision(pos1.x, pos1.y, hitbox1.width, hitbox1.height, pos2.x, pos2.y, hitbox2.width,
                                   hitbox2.height)) {
                HandleCollision(registry, entity1, entity2, collidable1.layer, collidable2.layer);
                if (!registry.isValid(entity1))
                    break; // Entity 1 destroyed, stop inner loop
            }
        }
    }
}

bool CollisionSystem::CheckAABBCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2,
                                         float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

bool CollisionSystem::ShouldCollide(component::CollisionLayer layer1, component::CollisionLayer layer2) {
    using CL = component::CollisionLayer;

    if (layer1 == CL::None || layer2 == CL::None) {
        return false;
    }

    if (layer1 == CL::Player && layer2 == CL::Enemy)
        return true;
    if (layer1 == CL::Enemy && layer2 == CL::Player)
        return true;

    if (layer1 == CL::Player && layer2 == CL::EnemyProjectile)
        return true;
    if (layer1 == CL::EnemyProjectile && layer2 == CL::Player)
        return true;

    if (layer1 == CL::Enemy && layer2 == CL::PlayerProjectile)
        return true;
    if (layer1 == CL::PlayerProjectile && layer2 == CL::Enemy)
        return true;

    if (layer1 == CL::Player && layer2 == CL::PowerUp)
        return true;
    if (layer1 == CL::PowerUp && layer2 == CL::Player)
        return true;

    if (layer1 == CL::Obstacle || layer2 == CL::Obstacle)
        return true;

    if (layer1 == CL::PlayerProjectile && layer2 == CL::EnemyProjectile)
        return true;
    if (layer1 == CL::EnemyProjectile && layer2 == CL::PlayerProjectile)
        return true;

    return false;
}

void CollisionSystem::HandleCollision(GameEngine::Registry& registry, GameEngine::entity_t entity1,
                                      GameEngine::entity_t entity2, component::CollisionLayer layer1,
                                      component::CollisionLayer layer2) {
    using CL = component::CollisionLayer;

    if ((layer1 == CL::Player && layer2 == CL::Enemy) || (layer1 == CL::Enemy && layer2 == CL::Player)) {
        auto player_entity = (layer1 == CL::Player) ? entity1 : entity2;

        if (registry.hasComponent<component::Health>(player_entity)) {
            auto& health = registry.getComponent<component::Health>(player_entity);
            if (health.hp <= 0) {
                return;
            }
            health.hp -= 10;

            if (health.hp <= 0) {
                if (!registry.hasComponent<component::Lives>(player_entity)) {
                    registry.destroyEntity(player_entity);
                }
            }
        }
    }

    if ((layer1 == CL::PlayerProjectile && layer2 == CL::Enemy) ||
        (layer1 == CL::Enemy && layer2 == CL::PlayerProjectile)) {
        auto projectile_entity = (layer1 == CL::PlayerProjectile) ? entity1 : entity2;
        auto enemy_entity = (layer1 == CL::Enemy) ? entity1 : entity2;

        std::size_t scorer_id = 0;
        if (registry.hasComponent<component::Projectile>(projectile_entity)) {
            scorer_id = registry.getComponent<component::Projectile>(projectile_entity).owner_id;
        }

        bool is_charged = false;
        if (registry.hasComponent<component::Tag>(projectile_entity)) {
            const auto& tag = registry.getComponent<component::Tag>(projectile_entity);
            if (tag.name.find("charge") != std::string::npos) {
                is_charged = true;
            }
        }

        if (!is_charged) {
            registry.destroyEntity(projectile_entity);
        }

        if (registry.hasComponent<component::Health>(enemy_entity)) {
            auto& health = registry.getComponent<component::Health>(enemy_entity);
            health.hp -= 25;

            // Trigger hit flash effect
            if (registry.hasComponent<component::HitFlash>(enemy_entity)) {
                auto& flash = registry.getComponent<component::HitFlash>(enemy_entity);
                flash.active = true;
                flash.timer = flash.duration;
            } else {
                registry.addComponent<component::HitFlash>(enemy_entity, 0.3f, 0.3f, true);
            }

            if (health.hp <= 0) {
                // Check if this is the Boss_1 - trigger stage cleared!
                bool is_boss = false;
                if (registry.hasComponent<component::Tag>(enemy_entity)) {
                    const auto& enemyTag = registry.getComponent<component::Tag>(enemy_entity);
                    if (enemyTag.name == "Boss_1") {
                        is_boss = true;
                        // Create StageCleared event - find or create a world entity
                        // Use entity 0 as the signal entity for stage cleared
                        auto world_entity = registry.createEntity();
                        registry.addComponent<component::StageCleared>(world_entity, 1);
                        std::cout << "[STAGE CLEARED] Boss_1 defeated!" << std::endl;
                    }
                }

                if (scorer_id != 0) {
                    auto scorer_entity = static_cast<GameEngine::entity_t>(scorer_id);
                    if (registry.isValid(scorer_entity) && registry.hasComponent<component::Score>(scorer_entity)) {
                        registry.addComponent<component::ScoreEvent>(scorer_entity, is_boss ? 1000 : 100);
                    }
                }

                // 30% chance to drop FORCE_POD item (not for boss)
                if (!is_boss && registry.hasComponent<component::Position>(enemy_entity)) {
                    auto& enemyPos = registry.getComponent<component::Position>(enemy_entity);
                    if (rand() % 100 < 30) {
                        // Check if scorer (player) already has max Force Pods (2)
                        int podCount = 0;
                        if (scorer_id != 0) {
                            auto view = registry.view<component::Tag, component::Parent>();
                            for (auto entity : view) {
                                auto& tag = view.get<component::Tag>(entity);
                                auto& parent = view.get<component::Parent>(entity);
                                if (tag.name == "ForcePod" && parent.ownerId == scorer_id) {
                                    podCount++;
                                }
                            }
                        }

                        if (podCount < 2) {
                            spawnForcePodItem(registry, enemyPos.x, enemyPos.y);
                        }
                    }
                }

                registry.destroyEntity(enemy_entity);
            }
        }
    }

    if ((layer1 == CL::EnemyProjectile && layer2 == CL::Player) ||
        (layer1 == CL::Player && layer2 == CL::EnemyProjectile)) {
        auto projectile_entity = (layer1 == CL::EnemyProjectile) ? entity1 : entity2;
        auto player_entity = (layer1 == CL::Player) ? entity1 : entity2;

        registry.destroyEntity(projectile_entity);

        if (registry.hasComponent<component::Health>(player_entity)) {
            auto& health = registry.getComponent<component::Health>(player_entity);
            if (health.hp <= 0) {
                return;
            }
            health.hp -= 20;

            if (health.hp <= 0) {
                if (!registry.hasComponent<component::Lives>(player_entity)) {
                    registry.destroyEntity(player_entity);
                }
            }
        }
    }

    if ((layer1 == CL::Player && layer2 == CL::PowerUp) || (layer1 == CL::PowerUp && layer2 == CL::Player)) {
        auto powerup_entity = (layer1 == CL::PowerUp) ? entity1 : entity2;
        auto player_entity = (layer1 == CL::Player) ? entity1 : entity2;

        // Check if this is a FORCE_POD power-up
        if (registry.hasComponent<component::PowerUpType>(powerup_entity)) {
            auto& type = registry.getComponent<component::PowerUpType>(powerup_entity);
            if (type.type == component::PowerUpTypeEnum::FORCE_POD) {
                spawnForcePodCompanion(registry, player_entity);
                registry.destroyEntity(powerup_entity);
                return;
            }
        }

        // Default: HEALTH power-up behavior
        if (registry.hasComponent<component::Health>(player_entity)) {
            auto& health = registry.getComponent<component::Health>(player_entity);
            health.hp += 30;
            if (health.hp > health.max_hp) {
                health.hp = health.max_hp;
            }
        }

        registry.destroyEntity(powerup_entity);
    }

    if ((layer1 == CL::Companion && layer2 == CL::Enemy) || (layer1 == CL::Enemy && layer2 == CL::Companion)) {
        auto enemy_entity = (layer1 == CL::Enemy) ? entity1 : entity2;
        // NOTE: We do not destroy the companion (Force Pod). It is invulnerable to body collisions.

        if (registry.hasComponent<component::Health>(enemy_entity)) {
            auto& health = registry.getComponent<component::Health>(enemy_entity);
            // Inflict massive damage to ensure instant kill per frame
            health.hp -= 100000;

            if (health.hp <= 0) {
                registry.destroyEntity(enemy_entity);
            }
        }
    }

    if ((layer1 == CL::Player && layer2 == CL::Obstacle) || (layer1 == CL::Obstacle && layer2 == CL::Player)) {
        auto player_entity = (layer1 == CL::Player) ? entity1 : entity2;
        auto obstacle_entity = (layer1 == CL::Obstacle) ? entity1 : entity2;

        if (registry.hasComponent<component::Position>(player_entity) &&
            registry.hasComponent<component::HitBox>(player_entity) &&
            registry.hasComponent<component::Position>(obstacle_entity) &&
            registry.hasComponent<component::HitBox>(obstacle_entity)) {

            auto& posP = registry.getComponent<component::Position>(player_entity);
            auto& boxP = registry.getComponent<component::HitBox>(player_entity);
            auto& posO = registry.getComponent<component::Position>(obstacle_entity);
            auto& boxO = registry.getComponent<component::HitBox>(obstacle_entity);

            float overlapX = std::min(posP.x + boxP.width, posO.x + boxO.width) - std::max(posP.x, posO.x);
            float overlapY = std::min(posP.y + boxP.height, posO.y + boxO.height) - std::max(posP.y, posO.y);

            if (overlapX < overlapY) {
                if (posP.x < posO.x)
                    posP.x -= overlapX;
                else
                    posP.x += overlapX;
            } else {
                if (posP.y < posO.y)
                    posP.y -= overlapY;
                else
                    posP.y += overlapY;
            }
        }
    }

    if ((layer1 == CL::PlayerProjectile && layer2 == CL::Obstacle) ||
        (layer1 == CL::Obstacle && layer2 == CL::PlayerProjectile)) {
        auto proj_entity = (layer1 == CL::PlayerProjectile) ? entity1 : entity2;
        registry.destroyEntity(proj_entity);
    }

    if ((layer1 == CL::PlayerProjectile && layer2 == CL::EnemyProjectile) ||
        (layer1 == CL::EnemyProjectile && layer2 == CL::PlayerProjectile)) {
        auto player_projectile = (layer1 == CL::PlayerProjectile) ? entity1 : entity2;
        auto enemy_projectile = (layer1 == CL::EnemyProjectile) ? entity1 : entity2;

        bool is_charged = false;
        if (registry.hasComponent<component::Tag>(player_projectile)) {
            const auto& tag = registry.getComponent<component::Tag>(player_projectile);
            if (tag.name.find("charge") != std::string::npos) {
                is_charged = true;
            }
        }

        if (!is_charged) {
            registry.destroyEntity(player_projectile);
        }
        registry.destroyEntity(enemy_projectile);
    }
}

void spawnForcePodItem(GameEngine::Registry& registry, float x, float y) {
    auto item = registry.createEntity();
    registry.addComponent<component::Position>(item, x, y);
    registry.addComponent<component::Velocity>(item, -50.0f, 0.0f); // Slow drift left
    registry.addComponent<component::Drawable>(item, "force_pod", 0, 0, 32, 32, 2.5f, 2.5f);
    registry.addComponent<component::PowerUpType>(item, component::PowerUpTypeEnum::FORCE_POD);
    registry.addComponent<component::Collidable>(item, component::CollisionLayer::PowerUp);
    registry.addComponent<component::HitBox>(item, 64.0f, 64.0f);
    registry.addComponent<component::Tag>(item, "ForcePodItem");
    registry.addComponent<component::SpawnEffect>(item);
    // NOTE: Do NOT add NetworkId here - let BroadcastSystem assign it
}

void spawnForcePodCompanion(GameEngine::Registry& registry, GameEngine::entity_t playerId) {
    auto pod = registry.createEntity();
    auto& playerPos = registry.getComponent<component::Position>(playerId);
    // Dynamic positioning based on existing pod count
    int podCount = 0;
    auto view = registry.view<component::Tag, component::Parent>();
    for (auto entity : view) {
        auto& tag = view.get<component::Tag>(entity);
        auto& parent = view.get<component::Parent>(entity);
        if (tag.name == "ForcePod" && parent.ownerId == static_cast<size_t>(playerId)) {
            podCount++;
        }
    }

    if (podCount >= 2) {
        return;
    }

    float offsetX = 0.0f;
    float offsetY = (podCount == 0) ? -50.0f : 100.0f; // 1st = Top (-50), 2nd = Bottom (+100) to avoid overlap
    registry.addComponent<component::Position>(pod, playerPos.x + offsetX, playerPos.y + offsetY);
    registry.addComponent<component::Velocity>(pod, 0.0f, 0.0f);
    registry.addComponent<component::Parent>(pod, static_cast<std::size_t>(playerId), offsetX, offsetY);
    registry.addComponent<component::Drawable>(pod, "force_pod", 0, 0, 32, 32, 2.5f, 2.5f);
    auto& weapon = registry.addComponent<component::Weapon>(pod);
    std::string pTag = (podCount == 0) ? "PodProjectile" : "PodProjectileRed";
    weapon.projectileTag = pTag;
    weapon.fireRate = 0.3f;
    weapon.projectileSpeed = 1000.0f;
    weapon.damage = 15.0f;
    weapon.spawnOffsetX =
        16.0f; // Center of pod (32 width * 2.5 scale / 2 ? No, offset is in world coords usually relative to pos)
    weapon.spawnOffsetY = 0.0f;
    registry.addComponent<component::Collidable>(pod, component::CollisionLayer::Companion);
    registry.addComponent<component::HitBox>(pod, 64.0f, 64.0f);
    registry.addComponent<component::Tag>(pod, "ForcePod");
    // NOTE: Do NOT add NetworkId here - let BroadcastSystem assign it
}

} // namespace rtype::ecs
