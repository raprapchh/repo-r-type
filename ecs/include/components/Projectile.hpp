#pragma once

#include <cstddef>

namespace rtype::ecs::component {

struct Projectile {
    float damage;
    float lifetime;
    std::size_t owner_id = 0;
};

} // namespace rtype::ecs::component
