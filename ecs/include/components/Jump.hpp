#pragma once

namespace rtype::ecs::component {

struct Jump {
    float strength = -600.0f;
    bool can_jump = false;
};

} // namespace rtype::ecs::component
