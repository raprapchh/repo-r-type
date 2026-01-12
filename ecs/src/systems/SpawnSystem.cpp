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
#include "../../include/components/MovementPattern.hpp"
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

    view.each([&registry, dt, maxX, maxY, this]([[maybe_unused]] auto entity, component::EnemySpawner& spawner) {
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
            if (spawner.currentWave == 0 && spawner.currentEnemyIndex == 0) {
                bool isBossLevel = false;
                if (!wave.enemies.empty()) {
                    if (wave.enemies[0].type.find("Boss") != std::string::npos) {
                        isBossLevel = true;
                    }
                }

                if (isBossLevel) {
                    if (!spawner.bossWarningActive && spawner.bossWarningTimer == 0.0f) {
                        spawner.bossWarningActive = true;
                        spawner.bossWarningTimer = 4.0f;

                        auto playerView = registry.view<component::Position, component::Tag>();
                        for (auto entity : playerView) {
                            const auto& tag = registry.getComponent<component::Tag>(entity);
                            if (tag.name == "Player") {
                                auto& pos = registry.getComponent<component::Position>(entity);
                                pos.x = 100.0f;
                                pos.y = 540.0f;
                            }
                        }
                    }
                }
            }

            if (spawner.bossWarningActive) {
                spawner.bossWarningTimer -= static_cast<float>(dt);
                if (spawner.bossWarningTimer <= 0.0f) {
                    spawner.bossWarningActive = false;
                    spawner.bossWarningTimer = 0.0f;
                } else {
                    return;
                }
            }

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
                } else if (tag == "Boss_1") {
                    offX = 0.0f;
                    offY = 100.0f;
                } else if (tag == "Monster_Wave_2_Left" || tag == "Monster_Wave_2_Right") {
                    offX = 0.0f;
                    offY = 0.0f;
                } else if (tag == "Boss_2") {
                    offX = 0.0f;
                    offY = 256.0f;
                }

                registry.addComponent<component::Position>(enemy, spawnX, spawnY);
                registry.addComponent<component::Velocity>(enemy, vx, vy);

                registry.addComponent<component::Tag>(enemy, tag);
                registry.addComponent<component::Collidable>(enemy, component::CollisionLayer::Enemy);

                auto& weapon = registry.addComponent<component::Weapon>(enemy);
                weapon.autoFire = true;
                weapon.fireRate = enemySpawn.fireRate;
                weapon.projectileSpeed = 500.0f;
                weapon.damage = 10.0f;
                weapon.projectileLifetime = 3.0f;
                weapon.spawnOffsetX = offX;
                weapon.spawnOffsetY = offY;
                weapon.directionX = dirX;
                weapon.directionY = dirY;

                if (tag == "Boss_1") {
                    registry.addComponent<component::HitBox>(enemy, 200.0f, 200.0f);
                    registry.addComponent<component::Health>(enemy, 1000, 1000);
                    registry.addComponent<component::MovementPattern>(
                        enemy, component::MovementPatternType::RandomVertical, 0.0f, 200.0f, 1.0f);
                    weapon.projectileTag = "Boss_1_Bayblade";
                    weapon.projectilePattern = component::MovementPatternType::Circular;
                    weapon.projectileAmplitude = 150.0f;
                    weapon.projectileFrequency = 5.0f;
                    weapon.damage = 50.0f;
                    weapon.fireRate = 0.2f;
                } else if (tag == "Boss_2") {
                    registry.addComponent<component::HitBox>(enemy, 256.0f, 256.0f);
                    registry.addComponent<component::Health>(enemy, 1000, 1000);
                    registry.addComponent<component::MovementPattern>(enemy, component::MovementPatternType::None, 0.0f,
                                                                      0.0f, 0.0f);
                    weapon.projectileTag = "Boss_2_Projectile";
                    weapon.projectilePattern = component::MovementPatternType::Circular;
                    weapon.projectileAmplitude = 100.0f;
                    weapon.projectileFrequency = 10.0f;
                    weapon.damage = 20.0f;
                    weapon.fireRate = 0.05f;
                } else if (tag == "Monster_Wave_2_Left" || tag == "Monster_Wave_2_Right") {
                    registry.addComponent<component::HitBox>(enemy, 100.0f, 100.0f);
                    registry.addComponent<component::Health>(enemy, 10, 10);
                    registry.addComponent<component::MovementPattern>(enemy, component::MovementPatternType::Sinusoidal,
                                                                      0.0f, 100.0f, 2.0f);
                    weapon.projectileTag = "Monster_0_Ball";
                    weapon.autoFire = false;
                } else {
                    registry.addComponent<component::HitBox>(enemy, 100.0f, 100.0f);
                    registry.addComponent<component::Health>(enemy, 5, 5);
                    weapon.projectileTag = "Monster_0_Ball";
                }

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
