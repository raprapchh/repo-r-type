#pragma once

#include "IRenderer.hpp"

namespace rtype::rendering {

class NullRenderer : public IRenderer {
  public:
    NullRenderer() = default;
    ~NullRenderer() override = default;

    void draw_sprite(const RenderData& data) override {
        (void)data;
    }

    void clear() override {
    }

    void display() override {
    }

    bool is_open() const override {
        return false;
    }

    bool get_texture_size(const std::string& texture_name, uint32_t& out_width, uint32_t& out_height) const override {
        (void)texture_name;
        (void)out_width;
        (void)out_height;
        return false;
    }
};

} // namespace rtype::rendering
