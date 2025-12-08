#include "../../include/systems/BoundarySystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/Registry.hpp"

namespace rtype::ecs {

void BoundarySystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;
    auto view = registry.view<component::Position>();

    view.each([&registry](const auto entity, component::Position& pos) {
        float width = 0.0f;
        float height = 0.0f;

        if (registry.hasComponent<component::HitBox>(static_cast<std::size_t>(entity))) {
            auto& hitbox = registry.getComponent<component::HitBox>(static_cast<std::size_t>(entity));
            width = hitbox.width;
            height = hitbox.height;
        }

        if (pos.x < 0)
            pos.x = 0;
        if (pos.x + width > 1920)
            pos.x = 1920 - width;

        if (pos.y < 0)
            pos.y = 0;
        if (pos.y + height > 1080)
            pos.y = 1080 - height;
    });
}

} // namespace rtype::ecs
