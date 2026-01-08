#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace rtype::rendering {

struct RenderData {
    float x, y;
    float scale_x, scale_y;
    std::string texture_name;
    uint32_t current_sprite;
    uint32_t sprite_index;
    uint16_t rect_x, rect_y;
    uint16_t rect_width, rect_height;
    uint32_t frame_count;
    uint8_t color_r, color_g, color_b, color_a;
    bool visible;
};

class IRenderer {
  public:
    virtual ~IRenderer() = default;

    virtual void draw_sprite(const RenderData& data) = 0;

    virtual void clear() = 0;

    virtual void display() = 0;

    virtual bool is_open() const = 0;

    virtual bool get_texture_size(const std::string& texture_name, uint32_t& out_width, uint32_t& out_height) const = 0;
};

} // namespace rtype::rendering
