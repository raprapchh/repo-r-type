#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace rtype::rendering {

/**
 * @struct RenderData
 * @brief Data structure for rendering a drawable entity
 * Contains all necessary information about a sprite to be rendered without SFML dependencies
 */
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

/**
 * @class IRenderer
 * @brief Abstract renderer interface for decoupling rendering backend from game logic
 *
 * Allows the ECS engine to remain headless by providing an abstraction layer.
 * Implementations can provide SFML, OpenGL, or no-op rendering.
 */
class IRenderer {
  public:
    virtual ~IRenderer() = default;

    /**
     * @brief Render a drawable entity
     * @param data The render data for the entity
     */
    virtual void draw_sprite(const RenderData& data) = 0;

    /**
     * @brief Clear the render target
     */
    virtual void clear() = 0;

    /**
     * @brief Display/swap the render target
     */
    virtual void display() = 0;

    /**
     * @brief Check if the render window is open/valid
     * @return true if renderer is ready to render, false otherwise
     */
    virtual bool is_open() const = 0;

    /**
     * @brief Get texture data for the given name
     * @param texture_name Name of the texture
     * @param out_width Output width of the texture
     * @param out_height Output height of the texture
     * @return true if texture exists, false otherwise
     */
    virtual bool get_texture_size(const std::string& texture_name, uint32_t& out_width, uint32_t& out_height) const = 0;
};

}
