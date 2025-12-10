#include "../include/NetworkSystem.hpp"
#include "../../shared/net/MessageData.hpp"
#include "../../ecs/include/components/Health.hpp"
#include "../../ecs/include/components/HitBox.hpp"
#include "../../ecs/include/components/Projectile.hpp"

namespace rtype::client {

NetworkSystem::NetworkSystem(uint32_t player_id) : player_id_(player_id) {
}

void NetworkSystem::push_packet(const rtype::net::Packet& packet) {
    std::lock_guard<std::mutex> lock(packet_queue_mutex_);
    packet_queue_.push(packet);
}

void NetworkSystem::set_player_id(uint32_t player_id) {
    player_id_ = player_id;
}

void NetworkSystem::update(GameEngine::Registry& registry, std::mutex& registry_mutex) {
    std::queue<rtype::net::Packet> packets_to_process;

    {
        std::lock_guard<std::mutex> lock(packet_queue_mutex_);
        packets_to_process = std::move(packet_queue_);
    }

    while (!packets_to_process.empty()) {
        rtype::net::Packet packet = packets_to_process.front();
        packets_to_process.pop();

        std::lock_guard<std::mutex> lock(registry_mutex);
        switch (static_cast<rtype::net::MessageType>(packet.header.message_type)) {
        case rtype::net::MessageType::EntitySpawn:
            handle_spawn(registry, packet);
            break;
        case rtype::net::MessageType::PlayerMove:
        case rtype::net::MessageType::EntityMove:
            handle_move(registry, packet);
            break;
        case rtype::net::MessageType::EntityDestroy:
            handle_destroy(registry, packet);
            break;
        default:
            break;
        }
    }
}

void NetworkSystem::handle_spawn(GameEngine::Registry& registry, const rtype::net::Packet& packet) {
    try {
        auto data = serializer_.deserialize_entity_spawn(packet);
        auto entity = registry.createEntity();

        registry.addComponent<rtype::ecs::component::NetworkId>(entity, data.entity_id);
        registry.addComponent<rtype::ecs::component::Position>(entity, data.position_x, data.position_y);
        registry.addComponent<rtype::ecs::component::Velocity>(entity, data.velocity_x, data.velocity_y);

        bool is_player = (data.entity_type == rtype::net::EntityType::PLAYER);
        bool is_local = (data.entity_id == player_id_);

        if (is_player) {
            uint32_t sprite_index = (data.entity_id - 1) % 4;
            registry.addComponent<rtype::ecs::component::Drawable>(entity, "player_ships", 0, 0, 33, 0, 5.0f, 5.0f, 0,
                                                                   0.1f, false, sprite_index, static_cast<uint32_t>(2));
            if (is_local) {
                registry.addComponent<rtype::ecs::component::Controllable>(entity, true);
            }
        } else if (data.entity_type == rtype::net::EntityType::ENEMY) {
            registry.addComponent<rtype::ecs::component::Drawable>(entity, "enemy_basic", static_cast<uint32_t>(0),
                                                                   static_cast<uint32_t>(0), 3.0f, 3.0f);
            registry.addComponent<rtype::ecs::component::Health>(entity, 100, 100);
            registry.addComponent<rtype::ecs::component::HitBox>(entity, 150.0f, 150.0f);
        } else if (data.entity_type == rtype::net::EntityType::PROJECTILE) {
            registry.addComponent<rtype::ecs::component::Drawable>(entity, "shot", 0, 0, 29, 33, 3.0f, 3.0f, 4, 0.05f,
                                                                   false);
            registry.addComponent<rtype::ecs::component::Projectile>(entity, 10.0f, 5.0f);
            registry.addComponent<rtype::ecs::component::HitBox>(entity, 87.0f, 99.0f);
        }

    } catch (const std::exception& e) {
    }
}

void NetworkSystem::handle_move(GameEngine::Registry& registry, const rtype::net::Packet& packet) {
    try {
        uint32_t entity_id = 0;
        float x = 0, y = 0;
        float vx = 0, vy = 0;

        if (static_cast<rtype::net::MessageType>(packet.header.message_type) == rtype::net::MessageType::PlayerMove) {
            auto data = serializer_.deserialize_player_move(packet);
            entity_id = data.player_id;
            x = data.position_x;
            y = data.position_y;
            vx = data.velocity_x;
            vy = data.velocity_y;
        } else {
            auto data = serializer_.deserialize_entity_move(packet);
            entity_id = data.entity_id;
            x = data.position_x;
            y = data.position_y;
            vx = data.velocity_x;
            vy = data.velocity_y;
        }

        auto view = registry.view<rtype::ecs::component::NetworkId>();
        bool found = false;

        for (auto entity : view) {
            GameEngine::entity_t entity_id_ecs = static_cast<GameEngine::entity_t>(entity);
            try {
                auto& net_id = registry.getComponent<rtype::ecs::component::NetworkId>(entity_id_ecs);
                if (net_id.id == entity_id) {
                    // Don't update local player's position/velocity - it's controlled locally
                    if (entity_id == player_id_) {
                        found = true;
                        break;
                    }

                    if (registry.hasComponent<rtype::ecs::component::Position>(entity_id_ecs)) {
                        auto& pos = registry.getComponent<rtype::ecs::component::Position>(entity_id_ecs);
                        pos.x = x;
                        pos.y = y;
                    }
                    if (registry.hasComponent<rtype::ecs::component::Velocity>(entity_id_ecs)) {
                        auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(entity_id_ecs);
                        vel.vx = vx;
                        vel.vy = vy;
                    }
                    found = true;
                    break;
                }
            } catch (const std::exception& e) {
                continue;
            }
        }

        if (!found) {
        }
    } catch (const std::exception& e) {
    }
}

void NetworkSystem::handle_destroy(GameEngine::Registry& registry, const rtype::net::Packet& packet) {
    try {
        auto data = serializer_.deserialize_entity_destroy(packet);

        auto view = registry.view<rtype::ecs::component::NetworkId>();

        for (auto entity : view) {
            GameEngine::entity_t entity_id_ecs = static_cast<GameEngine::entity_t>(entity);
            try {
                auto& net_id = registry.getComponent<rtype::ecs::component::NetworkId>(entity_id_ecs);
                if (net_id.id == data.entity_id) {
                    registry.destroyEntity(entity_id_ecs);
                    return;
                }
            } catch (const std::exception& e) {
                continue;
            }
        }
    } catch (const std::exception& e) {
    }
}

} // namespace rtype::client
