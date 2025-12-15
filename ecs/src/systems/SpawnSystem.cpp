#include "../../include/systems/SpawnSystem.hpp"
#include "../../include/components/EnemySpawner.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/MapBounds.hpp"
#include "../../include/components/Tag.hpp"
#include "../../include/components/Weapon.hpp"
#include "../../include/components/CollisionLayer.hpp"
#include "../../include/components/ScreenMode.hpp"
#include "../../../shared/utils/GameConfig.hpp"
#include <random>
#include <cmath>
#include <string>

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

    if (_levels.empty()) {
        _levels = config::getLevels();
    }

    auto view = registry.view<component::EnemySpawner>();

    view.each([&registry, dt, maxX, maxY, this](component::EnemySpawner& spawner) {
        spawner.waveTimer += static_cast<float>(dt);

        if (spawner.currentLevel >= static_cast<int>(_levels.size()))
            return;

        const auto& level = _levels[spawner.currentLevel];
        if (spawner.currentWave >= static_cast<int>(level.waves.size())) {
            spawner.currentLevel = 0;
            spawner.currentWave = 0;
            spawner.waveTimer = 0;
            spawner.currentEnemyIndex = 0;
            return;
        }

        const auto& wave = level.waves[spawner.currentWave];

        while (spawner.currentEnemyIndex < static_cast<int>(wave.enemies.size())) {
            const auto& enemySpawn = wave.enemies[spawner.currentEnemyIndex];
            if (spawner.waveTimer >= enemySpawn.spawnTime) {

                auto enemy = registry.createEntity();

                float spawnX = enemySpawn.x;
                float spawnY = enemySpawn.y;
                float vx = enemySpawn.vx;
                float vy = enemySpawn.vy;
                std::string tag = enemySpawn.type;

                float dirX = 0.0f;
                float dirY = 0.0f;
                if (std::abs(vx) > 0.001f || std::abs(vy) > 0.001f) {
                    float len = std::sqrt(vx * vx + vy * vy);
                    dirX = vx / len;
                    dirY = vy / len;
                } else {
                    dirX = -1.0f;
                    dirY = 0.0f;
                }

                float offX = 25.0f;
                float offY = 25.0f;

                if (tag == "Monster_0_Top") {
                    offX = 24.0f;
                    offY = 0.0f;
                } else if (tag == "Monster_0_Bot") {
                    offX = 25.0f;
                    offY = 20.0f;
                } else if (tag == "Monster_0_Left") {
                    offX = 0.0f;
                    offY = 20.0f;
                } else if (tag == "Monster_0_Right") {
                    offX = 50.0f;
                    offY = 20.0f;
                }

                registry.addComponent<component::Position>(enemy, spawnX, spawnY);
                registry.addComponent<component::Velocity>(enemy, vx, vy);
                registry.addComponent<component::HitBox>(enemy, 150.0f, 150.0f);
                registry.addComponent<component::Health>(enemy, 5, 5);
                registry.addComponent<component::Tag>(enemy, tag);
                registry.addComponent<component::Collidable>(enemy, component::CollisionLayer::Enemy);

                auto& weapon = registry.addComponent<component::Weapon>(enemy);
                weapon.autoFire = true;
                weapon.fireRate = enemySpawn.fireRate;
                weapon.projectileSpeed = 500.0f;
                weapon.damage = 10.0f;
                weapon.projectileLifetime = 3.0f;
                weapon.projectileTag = "Monster_0_Ball";
                weapon.spawnOffsetX = offX;
                weapon.spawnOffsetY = offY;
                weapon.directionX = dirX;
                weapon.directionY = dirY;

                spawner.currentEnemyIndex++;
            } else {
                break;
            }
        }

        if (spawner.waveTimer >= wave.duration) {
            spawner.currentWave++;
            spawner.waveTimer = 0;
            spawner.currentEnemyIndex = 0;

            if (spawner.currentWave >= static_cast<int>(level.waves.size())) {
                spawner.currentLevel++;
                spawner.currentWave = 0;
                if (spawner.currentLevel >= static_cast<int>(_levels.size())) {
                    spawner.currentLevel = 0;
                }
            }
        }
    });
}

} // namespace rtype::ecs
