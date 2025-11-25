#include "../include/Systems.hpp"
#include "../include/components/Position.hpp"
#include "../include/components/Velocity.hpp"
#include "../include/Registry.hpp"

namespace rtype::ecs {

void MovementSystem::update(GameEngine::Registry& registry) {
    auto view = registry.view<component::Position, component::Velocity>();

    view.each([](component::Position& pos, component::Velocity& vel) {
        pos.x += vel.vx;
        pos.y += vel.vy;
    });
}

} // namespace rtype::ecs
