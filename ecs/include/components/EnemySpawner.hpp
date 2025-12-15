#pragma once

namespace rtype::ecs::component {

struct EnemySpawner {
    float spawnInterval;
    float timeSinceLastSpawn;
    int currentLevel = 0;
    int currentWave = 0;
    int currentEnemyIndex = 0;
    float waveTimer = 0.0f;
};

} // namespace rtype::ecs::component
