#include "../../include/systems/SpawnSystem.hpp"
#include "../../include/components/EnemySpawner.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/MapBounds.hpp"
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
            std::uniform_real_distribution<float> dis(0.0f, maxY - 50.0f);
            float randomY = dis(gen);

            auto enemy = registry.createEntity();

            constexpr float ENEMY_WIDTH = 50.0f;
            constexpr float SPAWN_OFFSET = 10.0f;

            float spawnX = maxX + SPAWN_OFFSET;

            registry.addComponent<component::Position>(enemy, spawnX, randomY);
            registry.addComponent<component::Velocity>(enemy, -200.0f, 0.0f);
            registry.addComponent<component::HitBox>(enemy, ENEMY_WIDTH, 50.0f);
            registry.addComponent<component::Health>(enemy, 100, 100);
        }
    });
}

} // namespace rtype::ecs
