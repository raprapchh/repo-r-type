#pragma once

#include <memory>
#include "Renderer.hpp"
#include "Client.hpp"

namespace rtype::client {

enum class StateType { Menu, ModeSelection, Lobby, Game };

class StateManager;

class IState {
  public:
    virtual ~IState() = default;
    virtual void handle_input(Renderer& renderer, StateManager& state_manager) = 0;
    virtual void update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) = 0;
    virtual void render(Renderer& renderer) = 0;
    virtual void on_enter(Renderer& renderer, Client& client) = 0;
    virtual void on_exit(Renderer& renderer, Client& client) = 0;
    virtual StateType get_type() const = 0;
};

class StateManager {
  public:
    StateManager(Renderer& renderer, Client& client);
    ~StateManager() = default;

    void change_state(std::unique_ptr<IState> new_state);
    void handle_input();
    void update(float delta_time);
    void render();
    StateType get_current_state_type() const;

    Renderer& get_renderer() {
        return renderer_;
    }
    Client& get_client() {
        return client_;
    }

  private:
    Renderer& renderer_;
    Client& client_;
    std::unique_ptr<IState> current_state_;
};

} // namespace rtype::client
