#pragma once
#include "Registry.hpp"
#include <SFML/Graphics.hpp>

namespace rtype::ecs {

class LagometerSystem {
  public:
    void update(GameEngine::Registry& registry, double dt, sf::RenderWindow& window);

  private:
    void render_lagometer(GameEngine::Registry& registry, sf::RenderWindow& window);
};

} // namespace rtype::ecs
