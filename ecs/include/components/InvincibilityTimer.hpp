#pragma once

namespace rtype::ecs::component {

struct InvincibilityTimer {
    float timeRemaining;

    InvincibilityTimer() : timeRemaining(0.0f) {
    }

    InvincibilityTimer(float time) : timeRemaining(time) {
    }
};

} // namespace rtype::ecs::component
