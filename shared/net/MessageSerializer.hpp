#pragma once

#include "../interfaces/network/IMessageSerializer.hpp"
#include "MessageData.hpp"
#include "Packet.hpp"
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include <vector>
#include <cstdint>

namespace rtype::net {

class MessageSerializer : public IMessageSerializer {
  public:
    Packet serialize_player_move(const PlayerMoveData& data) override {
        Serializer serializer;
        serializer.write(data.player_id);
        serializer.write(data.position_x);
        serializer.write(data.position_y);
        serializer.write(data.velocity_x);
        serializer.write(data.velocity_y);
        return Packet(static_cast<uint16_t>(MessageType::PlayerMove), serializer.get_data());
    }

    PlayerMoveData deserialize_player_move(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        PlayerMoveData data;
        data.player_id = deserializer.read<uint32_t>();
        data.position_x = deserializer.read<float>();
        data.position_y = deserializer.read<float>();
        data.velocity_x = deserializer.read<float>();
        data.velocity_y = deserializer.read<float>();
        return data;
    }

    Packet serialize_player_shoot(const PlayerShootData& data) override {
        Serializer serializer;
        serializer.write(data.player_id);
        serializer.write(data.weapon_type);
        serializer.write(data.position_x);
        serializer.write(data.position_y);
        serializer.write(data.direction_x);
        serializer.write(data.direction_y);
        return Packet(static_cast<uint16_t>(MessageType::PlayerShoot), serializer.get_data());
    }

    PlayerShootData deserialize_player_shoot(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        PlayerShootData data;
        data.player_id = deserializer.read<uint32_t>();
        data.weapon_type = deserializer.read<uint16_t>();
        data.position_x = deserializer.read<float>();
        data.position_y = deserializer.read<float>();
        data.direction_x = deserializer.read<float>();
        data.direction_y = deserializer.read<float>();
        return data;
    }

    Packet serialize_player_join(const PlayerJoinData& data) override {
        Serializer serializer;
        serializer.write(data.player_id);
        return Packet(static_cast<uint16_t>(MessageType::PlayerJoin), serializer.get_data());
    }

    PlayerJoinData deserialize_player_join(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        PlayerJoinData data;
        data.player_id = deserializer.read<uint32_t>();
        return data;
    }

    Packet serialize_player_leave(const PlayerLeaveData& data) override {
        Serializer serializer;
        serializer.write(data.player_id);
        return Packet(static_cast<uint16_t>(MessageType::PlayerLeave), serializer.get_data());
    }

    PlayerLeaveData deserialize_player_leave(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        PlayerLeaveData data;
        data.player_id = deserializer.read<uint32_t>();
        return data;
    }

    Packet serialize_entity_spawn(const EntitySpawnData& data) override {
        Serializer serializer;
        serializer.write(data.entity_id);
        serializer.write(data.entity_type);
        serializer.write(data.sub_type);
        serializer.write(data.position_x);
        serializer.write(data.position_y);
        serializer.write(data.velocity_x);
        serializer.write(data.velocity_y);
        return Packet(static_cast<uint16_t>(MessageType::EntitySpawn), serializer.get_data());
    }

    EntitySpawnData deserialize_entity_spawn(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        EntitySpawnData data;
        data.entity_id = deserializer.read<uint32_t>();
        data.entity_type = deserializer.read<uint16_t>();
        data.sub_type = deserializer.read<uint16_t>();
        data.position_x = deserializer.read<float>();
        data.position_y = deserializer.read<float>();
        data.velocity_x = deserializer.read<float>();
        data.velocity_y = deserializer.read<float>();
        return data;
    }

    Packet serialize_entity_move(const EntityMoveData& data) override {
        Serializer serializer;
        serializer.write(data.entity_id);
        serializer.write(data.position_x);
        serializer.write(data.position_y);
        serializer.write(data.velocity_x);
        serializer.write(data.velocity_y);
        return Packet(static_cast<uint16_t>(MessageType::EntityMove), serializer.get_data());
    }

    EntityMoveData deserialize_entity_move(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        EntityMoveData data;
        data.entity_id = deserializer.read<uint32_t>();
        data.position_x = deserializer.read<float>();
        data.position_y = deserializer.read<float>();
        data.velocity_x = deserializer.read<float>();
        data.velocity_y = deserializer.read<float>();
        return data;
    }

    Packet serialize_entity_destroy(const EntityDestroyData& data) override {
        Serializer serializer;
        serializer.write(data.entity_id);
        serializer.write(data.reason);
        return Packet(static_cast<uint16_t>(MessageType::EntityDestroy), serializer.get_data());
    }

    EntityDestroyData deserialize_entity_destroy(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        EntityDestroyData data;
        data.entity_id = deserializer.read<uint32_t>();
        data.reason = deserializer.read<uint8_t>();
        return data;
    }

    Packet serialize_game_start(const GameStartData& data) override {
        Serializer serializer;
        serializer.write(data.session_id);
        serializer.write(data.level_id);
        serializer.write(data.player_count);
        serializer.write(data.difficulty);
        serializer.write(data.timestamp);
        return Packet(static_cast<uint16_t>(MessageType::GameStart), serializer.get_data());
    }

    GameStartData deserialize_game_start(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        GameStartData data;
        data.session_id = deserializer.read<uint32_t>();
        data.level_id = deserializer.read<uint16_t>();
        data.player_count = deserializer.read<uint8_t>();
        data.difficulty = deserializer.read<uint8_t>();
        data.timestamp = deserializer.read<uint32_t>();
        return data;
    }

    Packet serialize_game_state(const GameStateData& data) override {
        Serializer serializer;
        serializer.write(data.game_time);
        serializer.write(data.wave_number);
        serializer.write(data.enemies_remaining);
        serializer.write(data.score);
        serializer.write(data.game_state);
        serializer.write(data.padding[0]);
        serializer.write(data.padding[1]);
        serializer.write(data.padding[2]);
        return Packet(static_cast<uint16_t>(MessageType::GameState), serializer.get_data());
    }

    GameStateData deserialize_game_state(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        GameStateData data;
        data.game_time = deserializer.read<uint32_t>();
        data.wave_number = deserializer.read<uint16_t>();
        data.enemies_remaining = deserializer.read<uint16_t>();
        data.score = deserializer.read<uint32_t>();
        data.game_state = deserializer.read<uint8_t>();
        data.padding[0] = deserializer.read<uint8_t>();
        data.padding[1] = deserializer.read<uint8_t>();
        data.padding[2] = deserializer.read<uint8_t>();
        return data;
    }

    Packet serialize_ping(const PingPongData& data) override {
        Serializer serializer;
        serializer.write(data.timestamp);
        return Packet(static_cast<uint16_t>(MessageType::Ping), serializer.get_data());
    }

    Packet serialize_pong(const PingPongData& data) override {
        Serializer serializer;
        serializer.write(data.timestamp);
        return Packet(static_cast<uint16_t>(MessageType::Pong), serializer.get_data());
    }

    PingPongData deserialize_ping_pong(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        PingPongData data;
        data.timestamp = deserializer.read<uint64_t>();
        return data;
    }

    Packet serialize_map_resize(const MapResizeData& data) override {
        Serializer serializer;
        serializer.write(data.width);
        serializer.write(data.height);
        return Packet(static_cast<uint16_t>(MessageType::MapResize), serializer.get_data());
    }

    MapResizeData deserialize_map_resize(const Packet& packet) override {
        Deserializer deserializer(packet.body);
        MapResizeData data;
        data.width = deserializer.read<float>();
        data.height = deserializer.read<float>();
        return data;
    }
};

} // namespace rtype::net
