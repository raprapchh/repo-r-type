#include "../include/States.hpp"

namespace rtype::client {

StateManager::StateManager(Renderer& renderer, Client& client)
    : renderer_(renderer), client_(client), current_state_(nullptr) {
}

void StateManager::change_state(std::unique_ptr<IState> new_state) {
    pending_state_ = std::move(new_state);
}

void StateManager::process_pending_state_change() {
    if (pending_state_) {
        if (current_state_) {
            current_state_->on_exit(renderer_, client_);
        }
        current_state_ = std::move(pending_state_);
        if (current_state_) {
            current_state_->on_enter(renderer_, client_);
        }
    }
}

void StateManager::handle_input() {
    if (current_state_) {
        current_state_->handle_input(renderer_, *this);
    }
}

void StateManager::update(float delta_time) {
    process_pending_state_change();
    if (current_state_) {
        current_state_->update(renderer_, client_, *this, delta_time);
    }
}

void StateManager::render() {
    if (current_state_) {
        current_state_->render(renderer_, client_);
    }
}

StateType StateManager::get_current_state_type() const {
    if (current_state_) {
        return current_state_->get_type();
    }
    return StateType::Menu;
}

} // namespace rtype::client
