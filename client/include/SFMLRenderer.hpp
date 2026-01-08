#pragma once

#include "../../shared/interfaces/rendering/IRenderer.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>
#include <string>

namespace rtype::rendering {

class SFMLRenderer : public IRenderer {
  public:
    SFMLRenderer(sf::RenderWindow& window, std::unordered_map<std::string, sf::Texture>& textures);

    ~SFMLRenderer() override = default;

    void draw_sprite(const RenderData& data) override;

    void clear() override;

    void display() override;

    bool is_open() const override;

    bool get_texture_size(const std::string& texture_name, uint32_t& out_width, uint32_t& out_height) const override;

  private:
    sf::IntRect calculate_texture_rect(const RenderData& data, uint32_t texture_width, uint32_t texture_height) const;

    sf::RenderWindow& window_;
    std::unordered_map<std::string, sf::Texture>& textures_;
};

} // namespace rtype::rendering
