#include <catch2/catch_test_macros.hpp>
#include "Registry.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "components/Position.hpp"
#include "components/Velocity.hpp"
#include "components/HitBox.hpp"
#include "components/CollisionLayer.hpp"
#include "../shared/GameConstants.hpp"

TEST_CASE("Player vs Obstacle Stop Test", "[collision]") {
    GameEngine::Registry registry;
    rtype::ecs::CollisionSystem collisionSystem;
    rtype::ecs::MovementSystem movementSystem;

    auto player = registry.createEntity();
    registry.addComponent<rtype::ecs::component::Position>(player, 100.0f, 100.0f);
    registry.addComponent<rtype::ecs::component::Velocity>(player, 500.0f, 0.0f);
    registry.addComponent<rtype::ecs::component::HitBox>(
        player, rtype::constants::PLAYER_WIDTH * rtype::constants::PLAYER_SCALE,
        rtype::constants::PLAYER_HEIGHT * rtype::constants::PLAYER_SCALE);
    registry.addComponent<rtype::ecs::component::Collidable>(player, rtype::ecs::component::CollisionLayer::Player);

    auto obstacle = registry.createEntity();
    registry.addComponent<rtype::ecs::component::Position>(obstacle, 300.0f, 100.0f);
    registry.addComponent<rtype::ecs::component::HitBox>(obstacle, 288.0f, 288.0f);
    registry.addComponent<rtype::ecs::component::Collidable>(obstacle, rtype::ecs::component::CollisionLayer::Obstacle);

    double dt = 0.016;
    for (int i = 0; i < 60; ++i) {
        movementSystem.update(registry, dt);
        collisionSystem.update(registry, dt);
    }

    auto& finalPos = registry.getComponent<rtype::ecs::component::Position>(player);

    REQUIRE(finalPos.x < 300.0f);
    REQUIRE(finalPos.x <= 140.0f);
}
