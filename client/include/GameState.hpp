#pragma once

#include "States.hpp"
#include <chrono>

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
    Client* client_ = nullptr;
    bool game_over_ = false;
    bool all_players_dead_ = false;
    bool is_charging_ = false;
    std::chrono::steady_clock::time_point charge_start_time_;
};

} // namespace rtype::client
