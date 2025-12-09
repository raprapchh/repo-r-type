#pragma once

#include <cstdint>
#include "Protocol.hpp"

namespace rtype::net {

struct PlayerMoveData {
    uint32_t player_id;
    float position_x;
    float position_y;
    float velocity_x;
    float velocity_y;

    PlayerMoveData() : player_id(0), position_x(0.0f), position_y(0.0f), velocity_x(0.0f), velocity_y(0.0f) {
    }

    PlayerMoveData(uint32_t id, float x, float y, float vx, float vy)
        : player_id(id), position_x(x), position_y(y), velocity_x(vx), velocity_y(vy) {
    }
};

struct PlayerShootData {
    uint32_t player_id;
    uint16_t weapon_type;
    float position_x;
    float position_y;
    float direction_x;
    float direction_y;

    PlayerShootData()
        : player_id(0), weapon_type(0), position_x(0.0f), position_y(0.0f), direction_x(1.0f), direction_y(0.0f) {
    }

    PlayerShootData(uint32_t id, uint16_t weapon, float x, float y, float dx, float dy)
        : player_id(id), weapon_type(weapon), position_x(x), position_y(y), direction_x(dx), direction_y(dy) {
    }
};

struct PlayerJoinData {
    uint32_t player_id;

    PlayerJoinData() : player_id(0) {
    }

    explicit PlayerJoinData(uint32_t id) : player_id(id) {
    }
};

struct PlayerLeaveData {
    uint32_t player_id;

    PlayerLeaveData() : player_id(0) {
    }

    explicit PlayerLeaveData(uint32_t id) : player_id(id) {
    }
};

struct EntitySpawnData {
    uint32_t entity_id;
    uint16_t entity_type;
    uint16_t sub_type;
    float position_x;
    float position_y;
    float velocity_x;
    float velocity_y;

    EntitySpawnData()
        : entity_id(0), entity_type(0), sub_type(0), position_x(0.0f), position_y(0.0f), velocity_x(0.0f),
          velocity_y(0.0f) {
    }

    EntitySpawnData(uint32_t id, uint16_t type, uint16_t subtype, float x, float y, float vx, float vy)
        : entity_id(id), entity_type(type), sub_type(subtype), position_x(x), position_y(y), velocity_x(vx),
          velocity_y(vy) {
    }
};

struct EntityMoveData {
    uint32_t entity_id;
    float position_x;
    float position_y;
    float velocity_x;
    float velocity_y;

    EntityMoveData() : entity_id(0), position_x(0.0f), position_y(0.0f), velocity_x(0.0f), velocity_y(0.0f) {
    }

    EntityMoveData(uint32_t id, float x, float y, float vx, float vy)
        : entity_id(id), position_x(x), position_y(y), velocity_x(vx), velocity_y(vy) {
    }
};

struct EntityDestroyData {
    uint32_t entity_id;
    uint8_t reason;

    EntityDestroyData() : entity_id(0), reason(0) {
    }

    EntityDestroyData(uint32_t id, uint8_t destroy_reason) : entity_id(id), reason(destroy_reason) {
    }
};

struct GameStartData {
    uint32_t session_id;
    uint16_t level_id;
    uint8_t player_count;
    uint8_t difficulty;
    uint32_t timestamp;

    GameStartData() : session_id(0), level_id(0), player_count(0), difficulty(0), timestamp(0) {
    }

    GameStartData(uint32_t session, uint16_t level, uint8_t players, uint8_t diff, uint32_t time)
        : session_id(session), level_id(level), player_count(players), difficulty(diff), timestamp(time) {
    }
};

struct GameStateData {
    uint32_t game_time;
    uint16_t wave_number;
    uint16_t enemies_remaining;
    uint32_t score;
    uint8_t game_state;
    uint8_t lives;
    uint8_t padding[2];

    GameStateData()
        : game_time(0), wave_number(0), enemies_remaining(0), score(0), game_state(0), lives(3), padding{0, 0} {
    }

    GameStateData(uint32_t time, uint16_t wave, uint16_t enemies, uint32_t game_score, uint8_t state)
        : game_time(time), wave_number(wave), enemies_remaining(enemies), score(game_score), game_state(state),
          lives(3), padding{0, 0} {
    }
};

struct PingPongData {
    uint64_t timestamp;

    PingPongData() : timestamp(0) {
    }

    explicit PingPongData(uint64_t ts) : timestamp(ts) {
    }
};

namespace EntityType {
constexpr uint16_t ENEMY = 0;
constexpr uint16_t PROJECTILE = 1;
constexpr uint16_t POWERUP = 2;
constexpr uint16_t OBSTACLE = 3;
constexpr uint16_t PLAYER = 4;
} // namespace EntityType

namespace DestroyReason {
constexpr uint8_t TIMEOUT = 0;
constexpr uint8_t COLLISION = 1;
constexpr uint8_t KILLED = 2;
constexpr uint8_t COLLECTED = 3;
constexpr uint8_t OUT_OF_BOUNDS = 4;
} // namespace DestroyReason

namespace GameState {
constexpr uint8_t LOBBY = 0;
constexpr uint8_t PLAYING = 1;
constexpr uint8_t PAUSED = 2;
constexpr uint8_t GAME_OVER = 3;
} // namespace GameState

} // namespace rtype::net
