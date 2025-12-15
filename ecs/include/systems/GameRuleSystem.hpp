#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

class GameRuleSystem : public ISystem {
  public:
    GameRuleSystem(uint32_t player_id, bool& game_over, bool& all_players_dead);
    ~GameRuleSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;

  private:
    uint32_t player_id_;
    bool& game_over_;
    bool& all_players_dead_;
};

} // namespace rtype::ecs
