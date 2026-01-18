#pragma once

#include <cstdint>

namespace rtype::net {

/// @brief Network message types (OpCodes) for client-server communication
enum class MessageType : uint16_t {
    PlayerJoin = 1,    ///< Player connection request/response
    PlayerMove = 2,    ///< Player position/velocity update
    PlayerShoot = 3,   ///< Fire weapon action
    PlayerLeave = 4,   ///< Player disconnection
    EntitySpawn = 5,   ///< Create game entity
    EntityMove = 6,    ///< Update entity position
    EntityDestroy = 7, ///< Remove entity
    GameStart = 8,     ///< Begin game session
    GameState = 9,     ///< Periodic state snapshot
    Ping = 10,         ///< Latency probe
    Pong = 11,         ///< Latency response
    MapResize = 12,    ///< Viewport resize notification
    PlayerName = 13,
    ChatMessage = 14,      ///< Lobby chat message
    StageCleared = 15,     ///< Stage victory notification
    ListRooms = 16,        ///< Request list of available rooms
    RoomInfo = 17,         ///< Information about a room
    CreateRoom = 18,       ///< Create a new room
    JoinRoom = 19,         ///< Join an existing room
    LobbyUpdate = 20,      ///< Lobby state update (player count, player ID)
    RestartVote = 21,      ///< Player vote for game restart (play again or quit)
    RestartVoteStatus = 22 ///< Server broadcast of current vote status
};

} // namespace rtype::net
