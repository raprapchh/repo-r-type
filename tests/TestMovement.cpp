#include <catch2/catch_test_macros.hpp>
#include "Registry.hpp"
#include "components/Position.hpp"
#include "components/Velocity.hpp"
#include "systems/MovementSystem.hpp"

TEST_CASE("MovementSystem updates position based on velocity", "[MovementSystem]") {
    GameEngine::Registry registry;
    rtype::ecs::MovementSystem movementSystem;

    auto entity = registry.createEntity();
    registry.addComponent<rtype::ecs::component::Position>(entity, 0.0f, 0.0f);
    registry.addComponent<rtype::ecs::component::Velocity>(entity, 10.0f, 0.0f);

    movementSystem.update(registry, 1.0);

    auto& pos = registry.getComponent<rtype::ecs::component::Position>(entity);
    REQUIRE(pos.x == 10.0f);
    REQUIRE(pos.y == 0.0f);

    SECTION("Entity should move backwards with negative velocity") {
        auto backEntity = registry.createEntity();
        registry.addComponent<rtype::ecs::component::Position>(backEntity, 100.0f, 100.0f);
        registry.addComponent<rtype::ecs::component::Velocity>(backEntity, -10.0f, -20.0f);
        movementSystem.update(registry, 1.0);

        auto& pos = registry.getComponent<rtype::ecs::component::Position>(backEntity);

        REQUIRE(pos.x == 90.0f);
        REQUIRE(pos.y == 80.0f);
    }

    SECTION("Entity should NOT move if Delta Time is zero (Pause)") {
        auto fastEntity = registry.createEntity();
        registry.addComponent<rtype::ecs::component::Position>(fastEntity, 0.0f, 0.0f);
        registry.addComponent<rtype::ecs::component::Velocity>(fastEntity, 5000.0f, 5000.0f);

        movementSystem.update(registry, 0.0);

        auto& pos = registry.getComponent<rtype::ecs::component::Position>(fastEntity);
        REQUIRE(pos.x == 0.0f);
        REQUIRE(pos.y == 0.0f);
    }
}
