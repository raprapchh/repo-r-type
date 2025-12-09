#include <catch2/catch_test_macros.hpp>
#include "Registry.hpp"
#include "components/Position.hpp"
#include "components/Weapon.hpp"
#include "components/Projectile.hpp"
#include "systems/WeaponSystem.hpp"

TEST_CASE("WeaponSystem spawns projectiles", "[WeaponSystem]") {
    GameEngine::Registry registry;
    rtype::ecs::WeaponSystem weaponSystem;

    auto entity = registry.createEntity();
    registry.addComponent<rtype::ecs::component::Position>(entity, 100.0f, 100.0f);
    auto& weapon = registry.addComponent<rtype::ecs::component::Weapon>(entity);
    weapon.isShooting = true;
    weapon.timeSinceLastFire = 1.0f;
    weapon.fireRate = 0.5f;

    weaponSystem.update(registry, 0.1);

    bool projectileFound = false;
    auto view = registry.view<rtype::ecs::component::Projectile>();
    for (auto entity : view) {
        (void)entity;
        projectileFound = true;
        break;
    }
    REQUIRE(projectileFound);

    REQUIRE(weapon.timeSinceLastFire == 0.0f);
}

TEST_CASE("WeaponSystem respects fire rate", "[WeaponSystem]") {
    GameEngine::Registry registry;
    rtype::ecs::WeaponSystem weaponSystem;

    auto entity = registry.createEntity();
    registry.addComponent<rtype::ecs::component::Position>(entity, 100.0f, 100.0f);
    auto& weapon = registry.addComponent<rtype::ecs::component::Weapon>(entity);
    weapon.isShooting = true;
    weapon.timeSinceLastFire = 0.0f;
    weapon.fireRate = 0.5f;

    weaponSystem.update(registry, 0.1);

    bool projectileFound = false;
    auto view = registry.view<rtype::ecs::component::Projectile>();
    for (auto entity : view) {
        (void)entity;
        projectileFound = true;
        break;
    }
    REQUIRE_FALSE(projectileFound);

    REQUIRE(weapon.timeSinceLastFire == 0.1f);
}
