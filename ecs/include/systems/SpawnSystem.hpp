#pragma once

#include "interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"
#include "utils/LevelDefs.hpp"
#include <vector>

namespace rtype::ecs {

class SpawnSystem : public ISystem {
  public:
    ~SpawnSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;

  private:
    std::vector<config::Level> _levels;
};

} // namespace rtype::ecs
