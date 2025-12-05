#include "../../include/systems/InputSystem.hpp"
#include "../../include/components/Controllable.hpp"
#include "../../include/components/Velocity.hpp"

namespace rtype::ecs {

InputSystem::InputSystem(bool up, bool down, bool left, bool right, float speed)
    : up_pressed_(up), down_pressed_(down), left_pressed_(left), right_pressed_(right), speed_(speed) {
}

void InputSystem::update(GameEngine::Registry& registry, double /* dt */) {
    auto view = registry.view<component::Controllable, component::Velocity>();

    view.each([this](component::Controllable& ctrl, component::Velocity& vel) {
        if (!ctrl.is_local_player) {
            return;
        }

        vel.vx = 0.0f;
        vel.vy = 0.0f;

        if (left_pressed_) {
            vel.vx = -speed_;
        }
        if (right_pressed_) {
            vel.vx = speed_;
        }
        if (up_pressed_) {
            vel.vy = -speed_;
        }
        if (down_pressed_) {
            vel.vy = speed_;
        }
    });
}

} // namespace rtype::ecs

