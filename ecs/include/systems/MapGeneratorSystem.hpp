#pragma once

#include "Registry.hpp"

namespace rtype::ecs {

class MapGeneratorSystem {
  public:
    MapGeneratorSystem() = default;
    ~MapGeneratorSystem() = default;

    void update(GameEngine::Registry& registry, float view_center_y);

  private:
    float _last_platform_y = 600.0f;
    bool _initialized = false;
};

} // namespace rtype::ecs
