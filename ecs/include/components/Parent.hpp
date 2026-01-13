#pragma once

#include <cstddef>

namespace rtype::ecs::component {

/// @brief Links child entity to parent for position following
struct Parent {
    std::size_t ownerId;
    float offsetX = -50.0f;
    float offsetY = 0.0f;

    Parent() : ownerId(0) {
    }
    Parent(std::size_t owner, float ox = -50.0f, float oy = 0.0f) : ownerId(owner), offsetX(ox), offsetY(oy) {
    }
};

} // namespace rtype::ecs::component
