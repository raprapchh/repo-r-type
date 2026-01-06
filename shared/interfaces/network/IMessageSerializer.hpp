#pragma once

#include "../../net/MessageData.hpp"
#include "../../net/Packet.hpp"

namespace rtype::net {

class IMessageSerializer {
  public:
    virtual ~IMessageSerializer() = default;

    virtual Packet serialize_player_move(const PlayerMoveData& data) = 0;
    virtual PlayerMoveData deserialize_player_move(const Packet& packet) = 0;

    virtual Packet serialize_player_shoot(const PlayerShootData& data) = 0;
    virtual PlayerShootData deserialize_player_shoot(const Packet& packet) = 0;

    virtual Packet serialize_player_join(const PlayerJoinData& data) = 0;
    virtual PlayerJoinData deserialize_player_join(const Packet& packet) = 0;

    virtual Packet serialize_player_leave(const PlayerLeaveData& data) = 0;
    virtual PlayerLeaveData deserialize_player_leave(const Packet& packet) = 0;

    virtual Packet serialize_player_name(const PlayerNameData& data) = 0;
    virtual PlayerNameData deserialize_player_name(const Packet& packet) = 0;

    virtual Packet serialize_entity_spawn(const EntitySpawnData& data) = 0;
    virtual EntitySpawnData deserialize_entity_spawn(const Packet& packet) = 0;

    virtual Packet serialize_entity_move(const EntityMoveData& data) = 0;
    virtual EntityMoveData deserialize_entity_move(const Packet& packet) = 0;

    virtual Packet serialize_entity_destroy(const EntityDestroyData& data) = 0;
    virtual EntityDestroyData deserialize_entity_destroy(const Packet& packet) = 0;

    virtual Packet serialize_game_start(const GameStartData& data) = 0;
    virtual GameStartData deserialize_game_start(const Packet& packet) = 0;

    virtual Packet serialize_game_state(const GameStateData& data) = 0;
    virtual GameStateData deserialize_game_state(const Packet& packet) = 0;

    virtual Packet serialize_ping(const PingPongData& data) = 0;
    virtual Packet serialize_pong(const PingPongData& data) = 0;
    virtual PingPongData deserialize_ping_pong(const Packet& packet) = 0;

    virtual Packet serialize_map_resize(const MapResizeData& data) = 0;
    virtual MapResizeData deserialize_map_resize(const Packet& packet) = 0;
};

} // namespace rtype::net
