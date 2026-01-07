#include "../include/BroadcastSystem.hpp"
#include "../../shared/GameConstants.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Velocity.hpp"
#include "../../ecs/include/components/NetworkId.hpp"
#include "../../ecs/include/components/Tag.hpp"
#include "../../ecs/include/components/Projectile.hpp"
#include "../../ecs/include/components/Health.hpp"
#include "../../ecs/include/components/EnemySpawner.hpp"
#include "../../ecs/include/components/Score.hpp"
#include "../../ecs/include/components/Lives.hpp"
#include "../../ecs/include/components/MapBounds.hpp"
#include "../../shared/net/MessageData.hpp"
#include "../../shared/utils/Logger.hpp"

namespace rtype::server {

BroadcastSystem::BroadcastSystem(GameEngine::Registry& registry, UdpServer& udp_server,
                                 rtype::net::IProtocolAdapter& protocol_adapter,
                                 rtype::net::IMessageSerializer& message_serializer)
    : registry_(registry), udp_server_(udp_server), protocol_adapter_(protocol_adapter),
      message_serializer_(message_serializer) {
    next_network_id_ = 20000;
}

void BroadcastSystem::update(double dt, const std::map<std::string, ClientInfo>& clients) {
    if (clients.empty())
        return;

    broadcast_spawns(clients);
    broadcast_deaths(clients);
    broadcast_moves(clients);
    broadcast_game_state(clients, dt);

    last_known_entities_.clear();
    auto view = registry_.view<rtype::ecs::component::NetworkId>();
    for (auto entity : view) {
        auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
        last_known_entities_.insert(net_id.id);
    }
}

void BroadcastSystem::broadcast_packet(const std::vector<uint8_t>& data,
                                       const std::map<std::string, ClientInfo>& clients) {
    for (const auto& [key, client] : clients) {
        if (client.is_connected) {
            udp_server_.send(client.ip, client.port, data);
        }
    }
}

void BroadcastSystem::send_to_client(const std::vector<uint8_t>& data, const std::string& ip, uint16_t port) {
    udp_server_.send(ip, port, data);
}

void BroadcastSystem::broadcast_spawns(const std::map<std::string, ClientInfo>& clients) {
    std::vector<std::pair<rtype::net::EntitySpawnData, std::vector<uint8_t>>> spawns_to_send;
    std::vector<std::pair<size_t, uint32_t>> entities_to_add_network_id;

    auto view = registry_.view<rtype::ecs::component::Position, rtype::ecs::component::Velocity>();

    for (auto entity : view) {
        size_t entity_idx = static_cast<size_t>(entity);

        if (registry_.hasComponent<rtype::ecs::component::Tag>(entity_idx)) {
            const auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(entity_idx);
            if (tag.name == "Player")
                continue;
        }

        if (registry_.hasComponent<rtype::ecs::component::EnemySpawner>(entity_idx) ||
            registry_.hasComponent<rtype::ecs::component::MapBounds>(entity_idx)) {
            continue;
        }

        if (!registry_.hasComponent<rtype::ecs::component::NetworkId>(entity_idx)) {
            uint32_t net_id = next_network_id_++;
            entities_to_add_network_id.emplace_back(entity_idx, net_id);

            auto& pos = registry_.getComponent<rtype::ecs::component::Position>(entity_idx);
            auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(entity_idx);

            uint16_t type = rtype::net::EntityType::ENEMY;
            uint16_t sub_type = 0;

            if (registry_.hasComponent<rtype::ecs::component::Projectile>(entity_idx)) {
                type = rtype::net::EntityType::PROJECTILE;
            } else if (registry_.hasComponent<rtype::ecs::component::Tag>(entity_idx)) {
                const auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(entity_idx);
                if (tag.name == "Obstacle" || tag.name == "Obstacle_Floor") {
                    type = rtype::net::EntityType::OBSTACLE;
                    if (tag.name == "Obstacle_Floor")
                        sub_type = 1;
                }
            }

            if (registry_.hasComponent<rtype::ecs::component::Tag>(entity_idx)) {
                const auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(entity_idx);
                if (type == rtype::net::EntityType::ENEMY) {
                    if (tag.name == "Monster_0_Top")
                        sub_type = 1;
                    else if (tag.name == "Monster_0_Bot")
                        sub_type = 2;
                    else if (tag.name == "Monster_0_Left")
                        sub_type = 3;
                    else if (tag.name == "Monster_0_Right")
                        sub_type = 4;
                } else if (type == rtype::net::EntityType::PROJECTILE) {
                    if (tag.name == "Monster_0_Ball")
                        sub_type = 1;
                    else if (tag.name == "shot_death-charge1")
                        sub_type = 10;
                    else if (tag.name == "shot_death-charge2")
                        sub_type = 11;
                    else if (tag.name == "shot_death-charge3")
                        sub_type = 12;
                    else if (tag.name == "shot_death-charge4")
                        sub_type = 13;
                }
            }

            rtype::net::EntitySpawnData spawn_data(net_id, type, sub_type, pos.x, pos.y, vel.vx, vel.vy);
            rtype::net::Packet spawn_packet = message_serializer_.serialize_entity_spawn(spawn_data);
            auto serialized_spawn = protocol_adapter_.serialize(spawn_packet);
            spawns_to_send.emplace_back(spawn_data, serialized_spawn);
        }
    }

    for (const auto& [entity_id, net_id] : entities_to_add_network_id) {
        if (registry_.isValid(entity_id)) {
            registry_.addComponent<rtype::ecs::component::NetworkId>(entity_id, net_id);
        }
    }

    for (const auto& [spawn_data, serialized_spawn] : spawns_to_send) {
        broadcast_packet(serialized_spawn, clients);
        Logger::instance().info("Entity spawned and broadcasted: entity_id=" + std::to_string(spawn_data.entity_id));
    }
}

void BroadcastSystem::broadcast_deaths(const std::map<std::string, ClientInfo>& clients) {
    std::unordered_set<uint32_t> current_entities;
    auto view = registry_.view<rtype::ecs::component::NetworkId>();
    for (auto entity : view) {
        auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));
        current_entities.insert(net_id.id);
    }

    for (uint32_t old_id : last_known_entities_) {
        if (current_entities.find(old_id) == current_entities.end()) {
            rtype::net::EntityDestroyData destroy_data;
            destroy_data.entity_id = old_id;
            destroy_data.reason = rtype::net::DestroyReason::TIMEOUT;

            rtype::net::Packet destroy_packet = message_serializer_.serialize_entity_destroy(destroy_data);
            auto serialized_destroy = protocol_adapter_.serialize(destroy_packet);

            broadcast_packet(serialized_destroy, clients);
            Logger::instance().info("Entity destroyed broadcasted: entity_id=" + std::to_string(old_id));
        }
    }
}

void BroadcastSystem::broadcast_moves(const std::map<std::string, ClientInfo>& clients) {
    std::vector<std::vector<uint8_t>> player_moves;
    for (const auto& [key, client] : clients) {
        if (!client.is_connected || !registry_.isValid(client.entity_id))
            continue;

        if (registry_.hasComponent<rtype::ecs::component::Position>(client.entity_id) &&
            registry_.hasComponent<rtype::ecs::component::Velocity>(client.entity_id)) {

            auto& pos = registry_.getComponent<rtype::ecs::component::Position>(client.entity_id);
            auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(client.entity_id);

            rtype::net::PlayerMoveData move_data(client.player_id, pos.x, pos.y, vel.vx, vel.vy);
            rtype::net::Packet move_packet = message_serializer_.serialize_player_move(move_data);
            player_moves.push_back(protocol_adapter_.serialize(move_packet));
        }
    }

    for (const auto& data : player_moves) {
        broadcast_packet(data, clients);
    }

    std::vector<std::vector<uint8_t>> entity_moves;
    auto view =
        registry_
            .view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position, rtype::ecs::component::Velocity>();
    for (auto entity : view) {
        auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(static_cast<size_t>(entity));

        if (registry_.hasComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity))) {
            const auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(static_cast<size_t>(entity));
            if (tag.name == "Player")
                continue;
        }

        auto& pos = registry_.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
        auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(static_cast<size_t>(entity));

        rtype::net::EntityMoveData move_data(net_id.id, pos.x, pos.y, vel.vx, vel.vy);
        rtype::net::Packet move_packet = message_serializer_.serialize_entity_move(move_data);
        entity_moves.push_back(protocol_adapter_.serialize(move_packet));
    }

    for (const auto& data : entity_moves) {
        broadcast_packet(data, clients);
    }
}

void BroadcastSystem::broadcast_game_state(const std::map<std::string, ClientInfo>& clients, double elapsed_time) {
    rtype::net::GameStateData game_state_data;

    game_state_data.game_time = 0;

    int current_wave_display = 1;
    auto spawner_view = registry_.view<rtype::ecs::component::EnemySpawner>();
    for (auto entity : spawner_view) {
        auto& spawner = registry_.getComponent<rtype::ecs::component::EnemySpawner>(static_cast<size_t>(entity));
        current_wave_display = (spawner.currentLevel * 100) + (spawner.currentWave + 1);
        break;
    }
    game_state_data.wave_number = static_cast<uint16_t>(current_wave_display);

    for (const auto& [key, client] : clients) {
        if (!client.is_connected)
            continue;

        game_state_data.score = 0;
        game_state_data.lives = 0;

        if (registry_.isValid(client.entity_id)) {
            if (registry_.hasComponent<rtype::ecs::component::Score>(client.entity_id)) {
                game_state_data.score = registry_.getComponent<rtype::ecs::component::Score>(client.entity_id).value;
            }
            if (registry_.hasComponent<rtype::ecs::component::Lives>(client.entity_id)) {
                game_state_data.lives =
                    registry_.getComponent<rtype::ecs::component::Lives>(client.entity_id).remaining;
            }
        }

        rtype::net::Packet state_packet = message_serializer_.serialize_game_state(game_state_data);
        udp_server_.send(client.ip, client.port, protocol_adapter_.serialize(state_packet));
    }
}

void BroadcastSystem::send_initial_state(const std::string& ip, uint16_t port) {
    auto view = registry_.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position>();
    for (auto entity : view) {
        size_t entity_idx = static_cast<size_t>(entity);
        auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(entity_idx);
        auto& pos = registry_.getComponent<rtype::ecs::component::Position>(entity_idx);

        float vx = 0, vy = 0;
        if (registry_.hasComponent<rtype::ecs::component::Velocity>(entity_idx)) {
            auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(entity_idx);
            vx = vel.vx;
            vy = vel.vy;
        }

        uint16_t type = rtype::net::EntityType::ENEMY;
        uint16_t sub_type = 0;

        bool is_player = false;
        if (registry_.hasComponent<rtype::ecs::component::Tag>(entity_idx)) {
            const auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(entity_idx);
            if (tag.name == "Player")
                is_player = true;
            else if (tag.name == "Obstacle") {
                type = rtype::net::EntityType::OBSTACLE;
            } else if (tag.name == "Obstacle_Floor") {
                type = rtype::net::EntityType::OBSTACLE;
                sub_type = 1;
            } else if (tag.name == "Monster_0_Top")
                sub_type = 1;
            else if (tag.name == "Monster_0_Bot")
                sub_type = 2;
            else if (tag.name == "Monster_0_Left")
                sub_type = 3;
            else if (tag.name == "Monster_0_Right")
                sub_type = 4;

            if (tag.name == "Monster_0_Ball") {
                type = rtype::net::EntityType::PROJECTILE;
                sub_type = 1;
            } else if (tag.name == "shot_death-charge1") {
                type = rtype::net::EntityType::PROJECTILE;
                sub_type = 10;
            } else if (tag.name == "shot_death-charge2") {
                type = rtype::net::EntityType::PROJECTILE;
                sub_type = 11;
            } else if (tag.name == "shot_death-charge3") {
                type = rtype::net::EntityType::PROJECTILE;
                sub_type = 12;
            } else if (tag.name == "shot_death-charge4") {
                type = rtype::net::EntityType::PROJECTILE;
                sub_type = 13;
            }
        }

        if (registry_.hasComponent<rtype::ecs::component::Projectile>(entity_idx)) {
            type = rtype::net::EntityType::PROJECTILE;
        }

        if (is_player)
            continue;

        rtype::net::EntitySpawnData spawn_data(net_id.id, type, sub_type, pos.x, pos.y, vx, vy);
        rtype::net::Packet spawn_packet = message_serializer_.serialize_entity_spawn(spawn_data);
        send_to_client(protocol_adapter_.serialize(spawn_packet), ip, port);
    }
}

} // namespace rtype::server
