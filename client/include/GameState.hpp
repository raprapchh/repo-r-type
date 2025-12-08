#pragma once

#include "States.hpp"

namespace rtype::client {

class GameState : public IState {
  public:
    GameState();
    ~GameState() override = default;

    void handle_input(Renderer& renderer, StateManager& state_manager) override;
    void update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) override;
    void render(Renderer& renderer, Client& client) override;
    void on_enter(Renderer& renderer, Client& client) override;
    void on_exit(Renderer& renderer, Client& client) override;
    StateType get_type() const override {
        return StateType::Game;
    }

  private:
    bool shoot_requested_ = false;
};

} // namespace rtype::client
