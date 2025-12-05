#pragma once

#include <string>
#include <cstdint>

namespace rtype::ecs::component {

struct Drawable {
    std::string texture_name;
    uint32_t sprite_index;
    uint32_t animation_frame;
    float scale_x;
    float scale_y;

    Drawable() : texture_name(""), sprite_index(0), animation_frame(0), scale_x(1.0f), scale_y(1.0f) {
    }

    Drawable(const std::string& name, uint32_t index, uint32_t frame = 0, float sx = 1.0f, float sy = 1.0f)
        : texture_name(name), sprite_index(index), animation_frame(frame), scale_x(sx), scale_y(sy) {
    }
};

} // namespace rtype::ecs::component
