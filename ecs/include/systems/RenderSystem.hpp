#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"
#include <SFML/Graphics.hpp>

namespace rtype::ecs {

class RenderSystem : public ISystem {
  public:
    RenderSystem(sf::RenderWindow& window, std::unordered_map<std::string, sf::Texture>& textures);
    ~RenderSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;

  private:
    sf::RenderWindow& window_;
    std::unordered_map<std::string, sf::Texture>& textures_;
};

} // namespace rtype::ecs

