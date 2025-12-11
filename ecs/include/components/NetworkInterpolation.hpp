#pragma once
#include <chrono>

namespace rtype::ecs::component {

struct NetworkInterpolation {
    float target_x;
    float target_y;
    float target_vx;
    float target_vy;
    float interpolation_speed;
    std::chrono::steady_clock::time_point last_update_time;

    NetworkInterpolation()
        : target_x(0.0f), target_y(0.0f), target_vx(0.0f), target_vy(0.0f), interpolation_speed(10.0f),
          last_update_time(std::chrono::steady_clock::now()) {
    }

    NetworkInterpolation(float tx, float ty, float tvx, float tvy, float speed = 10.0f)
        : target_x(tx), target_y(ty), target_vx(tvx), target_vy(tvy), interpolation_speed(speed),
          last_update_time(std::chrono::steady_clock::now()) {
    }
};

} // namespace rtype::ecs::component

