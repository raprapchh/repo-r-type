#include "../../include/systems/MovementSystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"

namespace rtype::ecs {

void MovementSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Position, component::Velocity>();

    view.each([dt](component::Position& pos, component::Velocity& vel) {
        pos.x += vel.vx * dt;
        pos.y += vel.vy * dt;
    });
}

} // namespace rtype::ecs
