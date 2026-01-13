#pragma once

#include <cstdint>

namespace rtype::ecs::component {

enum class CollisionLayer : uint8_t {
    None = 0,
    Player = 1,
    Enemy = 2,
    PlayerProjectile = 3,
    EnemyProjectile = 4,
    PowerUp = 5,
    Obstacle = 6,
    Companion = 7
};

struct Collidable {
    CollisionLayer layer;
    bool is_active;

    Collidable() : layer(CollisionLayer::None), is_active(true) {
    }

    Collidable(CollisionLayer l) : layer(l), is_active(true) {
    }
};

} // namespace rtype::ecs::component
