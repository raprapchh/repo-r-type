#include "../../include/systems/BoundarySystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Weapon.hpp"
#include "../../include/components/Projectile.hpp"
#include "../../include/components/MapBounds.hpp"
#include "../../include/components/Tag.hpp"
#include "../../include/Registry.hpp"
#include <vector>

#include "../../shared/utils/GameConfig.hpp"

namespace rtype::ecs {

void BoundarySystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;

    float minX = rtype::config::MAP_MIN_X;
    float minY = rtype::config::MAP_MIN_Y;
    float maxX = rtype::config::MAP_MAX_X;
    float maxY = rtype::config::MAP_MAX_Y;

    try {
        auto mapBoundsView = registry.view<component::MapBounds>();
        for (auto entity : mapBoundsView) {
            auto& bounds = registry.getComponent<component::MapBounds>(static_cast<std::size_t>(entity));
            minX = bounds.minX;
            minY = bounds.minY;
            maxX = bounds.maxX;
            maxY = bounds.maxY;
            break;
        }
    } catch (const std::exception&) {
    }

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

        bool is_player = false;
        if (registry.hasComponent<component::Tag>(static_cast<std::size_t>(entity))) {
            const auto& tag = registry.getComponent<component::Tag>(static_cast<std::size_t>(entity));
            if (tag.name == "Player") {
                is_player = true;
            }
        }

        if (is_player) {
            if (pos.x < minX)
                pos.x = minX;
            if (pos.x + width > maxX)
                pos.x = maxX - width;
            if (pos.y < minY)
                pos.y = minY;
            if (pos.y + height > maxY)
                pos.y = maxY - height;
            return;
        }

        float buffer = 200.0f;
        if (pos.x < minX - buffer || pos.x > maxX + buffer || pos.y < minY - buffer || pos.y > maxY + buffer) {
            entities_to_destroy.push_back(static_cast<GameEngine::entity_t>(entity));
        }
    });

    for (auto entity : entities_to_destroy) {
        registry.destroyEntity(entity);
    }
}

} // namespace rtype::ecs