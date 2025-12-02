#include "../../include/systems/BoundarySystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/Registry.hpp"

namespace rtype::ecs {

void BoundarySystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;
    auto view = registry.view<component::Position>();

    view.each([](component::Position& pos) {
        if (pos.x < 0)
            pos.x = 0;
        if (pos.x > 1920)
            pos.x = 1920;
        if (pos.y < 0)
            pos.y = 0;
        if (pos.y > 1080)
            pos.y = 1080;
    });
}

} // namespace rtype::ecs
