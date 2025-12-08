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
    int rect_x;
    int rect_y;
    int rect_width;
    int rect_height;
    float animation_timer;
    int frame_count;
    float animation_speed;
    bool loop;

    Drawable()
        : texture_name(""), sprite_index(0), animation_frame(0), scale_x(1.0f), scale_y(1.0f), rect_x(0), rect_y(0),
          rect_width(0), rect_height(0), animation_timer(0.0f), frame_count(1), animation_speed(0.1f), loop(true) {
    }

    Drawable(const std::string& name, uint32_t index, uint32_t frame = 0, float sx = 1.0f, float sy = 1.0f)
        : texture_name(name), sprite_index(index), animation_frame(frame), scale_x(sx), scale_y(sy), rect_x(0),
          rect_y(0), rect_width(0), rect_height(0), animation_timer(0.0f), frame_count(1), animation_speed(0.1f),
          loop(true) {
    }

    Drawable(const std::string& name, int rx, int ry, int rw, int rh, float sx = 1.0f, float sy = 1.0f, int frames = 1,
             float speed = 0.1f, bool loop_anim = true)
        : texture_name(name), sprite_index(0), animation_frame(0), scale_x(sx), scale_y(sy), rect_x(rx), rect_y(ry),
          rect_width(rw), rect_height(rh), animation_timer(0.0f), frame_count(frames), animation_speed(speed),
          loop(loop_anim) {
    }
};

} // namespace rtype::ecs::component
