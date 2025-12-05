#pragma once

namespace rtype::ecs::component {

struct EnemySpawner {
    float spawnInterval;
    float timeSinceLastSpawn;
};

} // namespace rtype::ecs::component
