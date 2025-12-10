#pragma once

#include <string>
#include <cstdint>
#include "../../../shared/utils/Logger.hpp"

namespace rtype::ecs::component {

struct Drawable {
    std::string texture_name;
    uint32_t sprite_index;
    uint32_t animation_frame;
    uint32_t current_sprite;
    float scale_x;
    float scale_y;
    int rect_x;
    int rect_y;
    int rect_width;
    int rect_height;
    float animation_timer;
    int frame_count;
    float animation_speed;
    std::unordered_map<std::string, std::vector<uint32_t>> animation_sequences;
    std::string current_state = "";
    std::string last_state = "";
    uint32_t animation_index = 0;
    bool loop;

    Drawable()
        : texture_name(""), sprite_index(0), animation_frame(0), current_sprite(0), scale_x(1.0f), scale_y(1.0f),
          rect_x(0), rect_y(0), rect_width(0), rect_height(0), animation_timer(0.0f), frame_count(1),
          animation_speed(0.1f), loop(true) {
    }

    Drawable(const std::string& name, uint32_t index, uint32_t frame = 0, float sx = 1.0f, float sy = 1.0f)
        : texture_name(name), sprite_index(index), animation_frame(frame), current_sprite(frame), scale_x(sx),
          scale_y(sy), rect_x(0), rect_y(0), rect_width(0), rect_height(0), animation_timer(0.0f), frame_count(1),
          animation_speed(0.1f), loop(true) {
    }

    Drawable(const std::string& name, int rx, int ry, int rw, int rh, float sx = 1.0f, float sy = 1.0f, int frames = 1,
             float speed = 0.1f, bool loop_anim = true, uint32_t index = 0, uint32_t frame = 0)
        : texture_name(name), sprite_index(index), animation_frame(frame), current_sprite(frame), scale_x(sx),
          scale_y(sy), rect_x(rx), rect_y(ry), rect_width(rw), rect_height(rh), animation_timer(0.0f),
          frame_count(frames), animation_speed(speed), loop(loop_anim) {
        Logger::instance().debug("Drawable created with name: " + name + ", sx: " + std::to_string(sx) +
                                 ", sy: " + std::to_string(sy) + ", rx: " + std::to_string(rx) +
                                 ", ry: " + std::to_string(ry) + ", rw: " + std::to_string(rw) +
                                 ", rh: " + std::to_string(rh) + ", frames: " + std::to_string(frames) +
                                 ", speed: " + std::to_string(speed) + ", loop: " + std::to_string(loop) +
                                 ", index: " + std::to_string(index) + ", frame: " + std::to_string(frame));
    }
};

} // namespace rtype::ecs::component
