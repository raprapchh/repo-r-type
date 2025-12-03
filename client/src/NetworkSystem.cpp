#include "../include/NetworkSystem.hpp"
#include <iostream>

namespace rtype::client {

NetworkSystem::NetworkSystem() {
}

void NetworkSystem::push_packet(const rtype::net::Packet& packet) {
    packet_queue_.push(packet);
}

void NetworkSystem::update(GameEngine::Registry& registry) {
    while (!packet_queue_.empty()) {
        rtype::net::Packet packet = packet_queue_.front();
        packet_queue_.pop();

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

        std::cout << "Spawned entity " << data.entity_id << " at (" << data.position_x << ", " << data.position_y << ")"
                  << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error handling spawn packet: " << e.what() << std::endl;
    }
}

void NetworkSystem::handle_move(GameEngine::Registry& registry, const rtype::net::Packet& packet) {
    try {
        uint32_t entity_id = 0;
        float x = 0, y = 0;

        if (static_cast<rtype::net::MessageType>(packet.header.message_type) == rtype::net::MessageType::PlayerMove) {
            auto data = serializer_.deserialize_player_move(packet);
            entity_id = data.player_id;
            x = data.position_x;
            y = data.position_y;
        } else {
            auto data = serializer_.deserialize_entity_move(packet);
            entity_id = data.entity_id;
            x = data.position_x;
            y = data.position_y;
        }

        auto view = registry.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position>();
        bool found = false;

        for (auto entity : view) {
            auto& net_id = registry.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
            if (net_id.id == entity_id) {
                auto& pos = registry.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
                pos.x = x;
                pos.y = y;
                found = true;
                break;
            }
        }

        if (!found) {
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling move packet: " << e.what() << std::endl;
    }
}

void NetworkSystem::handle_destroy(GameEngine::Registry& registry, const rtype::net::Packet& packet) {
    try {
        auto data = serializer_.deserialize_entity_destroy(packet);
        auto view = registry.view<rtype::ecs::component::NetworkId>();

        for (auto entity : view) {
            auto& net_id = registry.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
            if (net_id.id == data.entity_id) {
                registry.destroyEntity(static_cast<size_t>(entity));
                std::cout << "Destroyed entity " << data.entity_id << std::endl;
                return;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling destroy packet: " << e.what() << std::endl;
    }
}

} // namespace rtype::client
