#pragma once

#include "../../ecs/include/Registry.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Tag.hpp"

namespace rtype::client {

class ParallaxSystem {
  public:
    ParallaxSystem(float speed = 100.0f) : speed_(speed) {
    }

    void update(GameEngine::Registry& registry, float delta_time) {
        auto view = registry.view<rtype::ecs::component::Position, rtype::ecs::component::Tag>();
        for (auto entity : view) {
            auto& tag = registry.getComponent<rtype::ecs::component::Tag>(static_cast<std::size_t>(entity));
            if (tag.name == "Obstacle" || tag.name == "Floor Obstacle") {
                auto& pos = registry.getComponent<rtype::ecs::component::Position>(static_cast<std::size_t>(entity));
                pos.x -= speed_ * delta_time;
            }
        }
    }

  private:
    float speed_;
};

} // namespace rtype::client
