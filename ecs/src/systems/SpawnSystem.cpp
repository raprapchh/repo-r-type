#include "../../include/systems/SpawnSystem.hpp"
#include "../../include/components/EnemySpawner.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/MapBounds.hpp"
#include "../../include/components/Tag.hpp"
#include "../../include/components/Weapon.hpp"
#include "../../shared/utils/GameConfig.hpp"
#include <random>

namespace rtype::ecs {

void SpawnSystem::update(GameEngine::Registry& registry, double dt) {
    float maxX = rtype::config::MAP_MAX_X;
    float maxY = rtype::config::MAP_MAX_Y;

    try {
        auto mapBoundsView = registry.view<component::MapBounds>();
        for (auto entity : mapBoundsView) {
            auto& bounds = registry.getComponent<component::MapBounds>(static_cast<std::size_t>(entity));
            maxX = bounds.maxX;
            maxY = bounds.maxY;
            break;
        }
    } catch (const std::exception&) {
    }

    auto view = registry.view<component::EnemySpawner>();

    view.each([&registry, dt, maxX, maxY](component::EnemySpawner& spawner) {
        spawner.timeSinceLastSpawn += static_cast<float>(dt);

        if (spawner.timeSinceLastSpawn >= spawner.spawnInterval) {
            spawner.timeSinceLastSpawn = 0.0f;

            static std::random_device rd;
            static std::mt19937 gen(rd());

            std::uniform_int_distribution<> dirDist(0, 3);
            int direction = dirDist(gen);

            float spawnX = 0, spawnY = 0;
            float vx = 0, vy = 0;
            std::string tag;
            float dirX = 0.0f, dirY = 0.0f;

            float offX = 25.0f;
            float offY = 25.0f;

            auto enemy = registry.createEntity();

            if (direction == 0) {
                std::uniform_real_distribution<float> xDist(0.0f, maxX - 50.0f);
                spawnX = xDist(gen);
                spawnY = -100.0f;
                vx = 0.0f;
                vy = 150.0f;
                tag = "Monster_0_Top";
                dirX = 0.0f;
                dirY = 1.0f;

                offX = 80.0f;
                offY = -100.0f;
            } else if (direction == 1) {
                std::uniform_real_distribution<float> xDist(0.0f, maxX - 50.0f);
                spawnX = xDist(gen);
                spawnY = maxY + 10.0f;
                vx = 0.0f;
                vy = -150.0f;
                tag = "Monster_0_Bot";
                dirX = 0.0f;
                dirY = -1.0f;

                offX = 6.0f;
                offY = 110.0f;
            } else if (direction == 2) {
                std::uniform_real_distribution<float> yDist(0.0f, maxY - 50.0f);
                spawnX = -60.0f;
                spawnY = yDist(gen);
                vx = 150.0f;
                vy = 0.0f;
                tag = "Monster_0_Left";
                dirX = 1.0f;
                dirY = 0.0f;

                offX = -100.0f;
                offY = 0.0f;
            } else {
                std::uniform_real_distribution<float> yDist(0.0f, maxY - 50.0f);
                spawnX = maxX + 100.0f;
                spawnY = yDist(gen);
                vx = -150.0f;
                vy = 0.0f;
                tag = "Monster_0_Right";
                dirX = -1.0f;
                dirY = 0.0f;

                offX = 110.0f;
                offY = 0.0f;
            }

            registry.addComponent<component::Position>(enemy, spawnX, spawnY);
            registry.addComponent<component::Velocity>(enemy, vx, vy);
            registry.addComponent<component::HitBox>(enemy, 50.0f, 50.0f);
            registry.addComponent<component::Health>(enemy, 100, 100);
            registry.addComponent<component::Tag>(enemy, tag);

            auto& weapon = registry.addComponent<component::Weapon>(enemy);
            weapon.autoFire = true;
            weapon.fireRate = 0.15f;
            weapon.projectileSpeed = 0.0f;
            weapon.damage = 10.0f;
            weapon.projectileLifetime = 3.0f;
            weapon.projectileTag = "Monster_0_Ball";
            weapon.spawnOffsetX = offX;
            weapon.spawnOffsetY = offY;
            weapon.directionX = dirX;
            weapon.directionY = dirY;
        }
    });
}

} // namespace rtype::ecs
