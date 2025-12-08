#include "../../include/systems/LivesSystem.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Lives.hpp"
#include "../../include/components/Position.hpp"
#include "../../shared/utils/Logger.hpp"

namespace rtype::ecs {

void LivesSystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;
    auto view = registry.view<component::Health, component::Lives, component::Position>();

    for (auto entity : view) {
        auto& health = view.get<component::Health>(entity);
        auto& lives = view.get<component::Lives>(entity);
        auto& pos = view.get<component::Position>(entity);

        if (health.hp <= 0) {
            lives.remaining--;
            Logger::instance().info("Player " + std::to_string(static_cast<std::size_t>(entity)) +
                                    " lost a life. Remaining: " + std::to_string(lives.remaining));

            if (lives.remaining > 0) {
                health.hp = health.max_hp;
                pos.x = 100.0f;
                pos.y = 100.0f;
                Logger::instance().info("Player respawned at (100, 100)");
            } else {
                Logger::instance().info("Game Over for player " + std::to_string(static_cast<std::size_t>(entity)));
                registry.destroyEntity(static_cast<std::size_t>(entity));
            }
        }
    }
}

} // namespace rtype::ecs
