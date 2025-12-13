#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"
#include <SFML/Graphics.hpp>

namespace rtype::client {
class AccessibilityManager;
}

namespace rtype::ecs {

class RenderSystem : public ISystem {
  public:
    RenderSystem(sf::RenderWindow& window, std::unordered_map<std::string, sf::Texture>& textures,
                 rtype::client::AccessibilityManager* accessibility_mgr = nullptr);
    ~RenderSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;

  private:
    sf::RenderWindow& window_;
    std::unordered_map<std::string, sf::Texture>& textures_;
    rtype::client::AccessibilityManager* accessibility_manager_;
};

} // namespace rtype::ecs
