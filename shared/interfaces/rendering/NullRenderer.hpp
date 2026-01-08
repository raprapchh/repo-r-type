#pragma once

#include "../../shared/interfaces/rendering/IRenderer.hpp"

namespace rtype::rendering {

/**
 * @class NullRenderer
 * @brief No-operation renderer for headless mode
 * 
 * This renderer is used when the ECS engine runs without any graphics output.
 * It implements all IRenderer methods as no-ops, allowing the core engine to function
 * independently of any rendering backend.
 * 
 * Useful for:
 * - Server-side physics/logic simulation
 * - Automated testing
 * - Headless game servers
 */
class NullRenderer : public IRenderer {
  public:
    NullRenderer() = default;
    ~NullRenderer() override = default;

    /**
     * @brief No-op draw_sprite implementation
     */
    void draw_sprite(const RenderData& data) override {
        (void)data; // Suppress unused parameter warning
    }

    /**
     * @brief No-op clear implementation
     */
    void clear() override {}

    /**
     * @brief No-op display implementation
     */
    void display() override {}

    /**
     * @brief Always returns false to indicate no rendering is available
     */
    bool is_open() const override {
        return false;
    }

    /**
     * @brief No-op texture size query - always returns false
     */
    bool get_texture_size(const std::string& texture_name, uint32_t& out_width,
                         uint32_t& out_height) const override {
        (void)texture_name;
        (void)out_width;
        (void)out_height;
        return false;
    }
};

} // namespace rtype::rendering
