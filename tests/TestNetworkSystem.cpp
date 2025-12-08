#include <catch2/catch_test_macros.hpp>
#include "NetworkSystem.hpp"
#include "Registry.hpp"
#include "components/Position.hpp"
#include "components/NetworkId.hpp"
#include "../../shared/net/MessageSerializer.hpp"
#include <mutex>

TEST_CASE("NetworkSystem handles Spawn, Move, and Destroy packets", "[NetworkSystem]") {
    GameEngine::Registry registry;
    std::mutex registry_mutex;
    rtype::client::NetworkSystem networkSystem;
    rtype::net::MessageSerializer serializer;

    SECTION("Spawn Entity") {
        rtype::net::EntitySpawnData spawnData;
        spawnData.entity_id = 100;
        spawnData.position_x = 50.0f;
        spawnData.position_y = 60.0f;

        rtype::net::Packet packet = serializer.serialize_entity_spawn(spawnData);
        networkSystem.push_packet(packet);
        networkSystem.update(registry, registry_mutex);

        bool found = false;
        auto view = registry.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position>();
        for (auto entity : view) {
            auto& netId = registry.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
            if (netId.id == 100) {
                auto& pos = registry.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
                REQUIRE(pos.x == 50.0f);
                REQUIRE(pos.y == 60.0f);
                found = true;
            }
        }
        REQUIRE(found);
    }

    SECTION("Move Entity") {
        rtype::net::EntitySpawnData spawnData;
        spawnData.entity_id = 101;
        spawnData.position_x = 10.0f;
        spawnData.position_y = 10.0f;
        networkSystem.push_packet(serializer.serialize_entity_spawn(spawnData));
        networkSystem.update(registry, registry_mutex);

        rtype::net::EntityMoveData moveData;
        moveData.entity_id = 101;
        moveData.position_x = 20.0f;
        moveData.position_y = 20.0f;
        networkSystem.push_packet(serializer.serialize_entity_move(moveData));
        networkSystem.update(registry, registry_mutex);

        bool found = false;
        auto view = registry.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position>();
        for (auto entity : view) {
            auto& netId = registry.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
            if (netId.id == 101) {
                auto& pos = registry.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
                REQUIRE(pos.x == 20.0f);
                REQUIRE(pos.y == 20.0f);
                found = true;
            }
        }
        REQUIRE(found);
    }

    SECTION("Destroy Entity") {
        rtype::net::EntitySpawnData spawnData;
        spawnData.entity_id = 102;
        networkSystem.push_packet(serializer.serialize_entity_spawn(spawnData));
        networkSystem.update(registry, registry_mutex);

        bool found = false;
        auto view = registry.view<rtype::ecs::component::NetworkId>();
        for (auto entity : view) {
            auto& netId = registry.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
            if (netId.id == 102)
                found = true;
        }
        REQUIRE(found);

        rtype::net::EntityDestroyData destroyData;
        destroyData.entity_id = 102;
        networkSystem.push_packet(serializer.serialize_entity_destroy(destroyData));
        networkSystem.update(registry, registry_mutex);
        found = false;
        auto viewAfter = registry.view<rtype::ecs::component::NetworkId>();
        for (auto entity : viewAfter) {
            auto& netId = registry.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
            if (netId.id == 102)
                found = true;
        }
        REQUIRE_FALSE(found);
    }
}
