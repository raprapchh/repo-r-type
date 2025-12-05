#include "../../include/systems/SpawnSystem.hpp"
#include "../../include/components/EnemySpawner.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include <random>

namespace rtype::ecs {

void SpawnSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::EnemySpawner>();

    view.each([&registry, dt](component::EnemySpawner& spawner) {
        spawner.timeSinceLastSpawn += static_cast<float>(dt);

        if (spawner.timeSinceLastSpawn >= spawner.spawnInterval) {
            spawner.timeSinceLastSpawn = 0.0f;

            // Random Y position between 0 and 1080
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(0.0f, 1080.0f);
            float randomY = dis(gen);

            // Create enemy entity
            auto enemy = registry.createEntity();

            // Add components to the enemy
            registry.addComponent<component::Position>(enemy, 1920.0f, randomY);
            registry.addComponent<component::Velocity>(enemy, -200.0f, 0.0f);
            registry.addComponent<component::HitBox>(enemy, 50.0f, 50.0f);
            registry.addComponent<component::Health>(enemy, 100, 100);
        }
    });
}

} // namespace rtype::ecs
