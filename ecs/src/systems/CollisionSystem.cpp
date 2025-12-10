#include "../../include/systems/CollisionSystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/CollisionLayer.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Projectile.hpp"
#include "../../include/components/Score.hpp"
#include "../../include/components/Lives.hpp"

namespace rtype::ecs {

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

        auto entt_entity1 = static_cast<entt::entity>(entity1);
        auto& pos1 = view.get<component::Position>(entt_entity1);
        auto& hitbox1 = view.get<component::HitBox>(entt_entity1);
        auto& collidable1 = view.get<component::Collidable>(entt_entity1);

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

            auto entt_entity2 = static_cast<entt::entity>(entity2);
            auto& pos2 = view.get<component::Position>(entt_entity2);
            auto& hitbox2 = view.get<component::HitBox>(entt_entity2);
            auto& collidable2 = view.get<component::Collidable>(entt_entity2);

            if (!collidable2.is_active) {
                continue;
            }

            if (!ShouldCollide(collidable1.layer, collidable2.layer)) {
                continue;
            }

            if (CheckAABBCollision(pos1.x, pos1.y, hitbox1.width, hitbox1.height, pos2.x, pos2.y, hitbox2.width,
                                   hitbox2.height)) {
                HandleCollision(registry, entity1, entity2, collidable1.layer, collidable2.layer);
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

        registry.destroyEntity(projectile_entity);

        if (registry.hasComponent<component::Health>(enemy_entity)) {
            auto& health = registry.getComponent<component::Health>(enemy_entity);
            health.hp -= 25;

            if (health.hp <= 0) {
                if (scorer_id != 0) {
                    auto scorer_entity = static_cast<GameEngine::entity_t>(scorer_id);
                    if (registry.isValid(scorer_entity) && registry.hasComponent<component::Score>(scorer_entity)) {
                        registry.addComponent<component::ScoreEvent>(scorer_entity, 100);
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

        if (registry.hasComponent<component::Health>(player_entity)) {
            auto& health = registry.getComponent<component::Health>(player_entity);
            health.hp += 30;
            if (health.hp > health.max_hp) {
                health.hp = health.max_hp;
            }
        }

        registry.destroyEntity(powerup_entity);
    }
}

} // namespace rtype::ecs
