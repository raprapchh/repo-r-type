#pragma once

namespace rtype::ecs::component {

enum class MovementPatternType {
    None = 0,
    ZigZag = 1,
    Straight = 2,
    Sinusoidal = 3,
    Hover = 4,
    Circular = 5,
    RandomVertical = 6
};

struct MovementPattern {
    MovementPatternType type;
    float timer = 0.0f;
    float amplitude = 0.0f;
    float frequency = 0.0f;
};

} // namespace rtype::ecs::component
