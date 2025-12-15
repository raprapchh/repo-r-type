#include "../../include/systems/InterpolationSystem.hpp"
#include "../../include/components/NetworkInterpolation.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include <chrono>
#include <algorithm>

namespace rtype::ecs {

void InterpolationSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::NetworkInterpolation, component::Position, component::Velocity>();
    auto now = std::chrono::steady_clock::now();
    constexpr auto timeout = std::chrono::milliseconds(500);

    for (auto entity : view) {
        auto& interp = registry.getComponent<component::NetworkInterpolation>(static_cast<std::size_t>(entity));
        auto& pos = registry.getComponent<component::Position>(static_cast<std::size_t>(entity));
        auto& vel = registry.getComponent<component::Velocity>(static_cast<std::size_t>(entity));

        auto time_since_update = std::chrono::duration_cast<std::chrono::milliseconds>(now - interp.last_update_time);

        if (time_since_update > timeout) {
            pos.x = interp.target_x;
            pos.y = interp.target_y;
            vel.vx = 0.0f;
            vel.vy = 0.0f;
        } else {
            float lerp_factor = std::min(1.0f, interp.interpolation_speed * static_cast<float>(dt));
            pos.x = pos.x + (interp.target_x - pos.x) * lerp_factor;
            pos.y = pos.y + (interp.target_y - pos.y) * lerp_factor;
            vel.vx = interp.target_vx;
            vel.vy = interp.target_vy;
        }
    }
}

} // namespace rtype::ecs
