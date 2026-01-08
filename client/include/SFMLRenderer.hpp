#pragma once

#include "../../shared/interfaces/rendering/IRenderer.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>
#include <string>

namespace rtype::rendering {

/**
 * @class SFMLRenderer
 * @brief SFML-based implementation of IRenderer
 * 
 * Encapsulates all SFML-specific rendering logic, isolating it from the core ECS engine.
 * This allows the engine to remain headless-capable while providing concrete SFML rendering.
 */
class SFMLRenderer : public IRenderer {
  public:
    /**
     * @brief Construct an SFML renderer with an existing window
     * @param window Reference to an SFML RenderWindow
     * @param textures Reference to loaded textures map
     */
    SFMLRenderer(sf::RenderWindow& window, std::unordered_map<std::string, sf::Texture>& textures);
    
    ~SFMLRenderer() override = default;

    /**
     * @brief Draw a sprite to the render window
     * @param data The render data for the sprite
     */
    void draw_sprite(const RenderData& data) override;

    /**
     * @brief Clear the render window
     */
    void clear() override;

    /**
     * @brief Display the render window
     */
    void display() override;

    /**
     * @brief Check if the render window is open
     * @return true if window is open
     */
    bool is_open() const override;

    /**
     * @brief Get texture dimensions
     * @param texture_name Name of the texture
     * @param out_width Output width
     * @param out_height Output height
     * @return true if texture exists, false otherwise
     */
    bool get_texture_size(const std::string& texture_name, uint32_t& out_width, 
                         uint32_t& out_height) const override;

  private:
    /**
     * @brief Calculate the correct texture rectangle based on entity type and animation frame
     * @param data The render data
     * @param texture_width Width of the texture
     * @param texture_height Height of the texture
     * @return An SFML IntRect for the texture coordinates
     */
    sf::IntRect calculate_texture_rect(const RenderData& data, uint32_t texture_width, 
                                      uint32_t texture_height) const;

    sf::RenderWindow& window_;
    std::unordered_map<std::string, sf::Texture>& textures_;
};

} // namespace rtype::rendering
