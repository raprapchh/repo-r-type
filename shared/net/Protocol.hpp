#pragma once

#include <cstdint>

namespace rtype::net {

enum class MessageType : uint16_t {
    PlayerJoin = 1,
    PlayerMove = 2,
    PlayerShoot = 3,
    PlayerLeave = 4,
    EntitySpawn = 5,
    EntityMove = 6,
    EntityDestroy = 7,
    GameStart = 8,
    GameState = 9,
    Ping = 10,
    Pong = 11
};

} // namespace rtype::net
