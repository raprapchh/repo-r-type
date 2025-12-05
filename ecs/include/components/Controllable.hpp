#pragma once

namespace rtype::ecs::component {

struct Controllable {
    bool is_local_player;

    Controllable() : is_local_player(true) {
    }

    explicit Controllable(bool local) : is_local_player(local) {
    }
};

} // namespace rtype::ecs::component

