#pragma once

#include "interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"
#include <SFML/Graphics.hpp>

namespace rtype::ecs {

/// @brief System to render UI overlay elements (TextDrawable with UITag)
class UIRenderSystem : public ISystem {
  public:
    explicit UIRenderSystem(sf::RenderWindow* window = nullptr, sf::Font* font = nullptr);
    ~UIRenderSystem() override = default;

    void update(GameEngine::Registry& registry, double dt) override;

    void setWindow(sf::RenderWindow* window) {
        window_ = window;
    }

    void setFont(sf::Font* font) {
        font_ = font;
    }

  private:
    sf::RenderWindow* window_;
    sf::Font* font_;
};

} // namespace rtype::ecs
