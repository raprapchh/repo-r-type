#include "../../include/systems/BoundarySystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Weapon.hpp"
#include "../../include/Registry.hpp"

namespace rtype::ecs {

void BoundarySystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;
    auto view = registry.view<component::Position>();

    std::vector<GameEngine::entity_t> entities_to_destroy;

    view.each([&](const auto entity, component::Position& pos) {
        float width = 0.0f;
        float height = 0.0f;

        if (registry.hasComponent<component::HitBox>(static_cast<std::size_t>(entity))) {
            auto& hitbox = registry.getComponent<component::HitBox>(static_cast<std::size_t>(entity));
            width = hitbox.width;
            height = hitbox.height;
        }

        // Check if entity is an enemy (has Health, no Weapon)
        bool is_enemy = registry.hasComponent<component::Health>(static_cast<std::size_t>(entity)) &&
                        !registry.hasComponent<component::Weapon>(static_cast<std::size_t>(entity));

        // Destroy enemies when they go off-screen to the left
        if (is_enemy && pos.x + width < 0) {
            entities_to_destroy.push_back(static_cast<GameEngine::entity_t>(entity));
            return;
        }

        // Don't clamp enemies - let them move freely until destroyed
        if (is_enemy) {
            return;
        }

        // For players and other entities, clamp to boundaries
        if (pos.x < 0)
            pos.x = 0;
        if (pos.x + width > 1920)
            pos.x = 1920 - width;

        if (pos.y < 0)
            pos.y = 0;
        if (pos.y + height > 1080)
            pos.y = 1080 - height;
    });

    // Destroy all marked entities
    for (auto entity : entities_to_destroy) {
        registry.destroyEntity(entity);
    }
}

} // namespace rtype::ecs
