# R-Type Protocol Specification (RFC-like)

## 1. Introduction

This document defines the strict binary communication protocol for the R-Type project. The protocol is used for both UDP (gameplay) and TCP (lobby) communication. All multi-byte integer values are transmitted in little-endian byte order.

## 2. Packet Structure

All communication is based on a single, unified packet structure. Each packet consists of a fixed-size header followed by a variable-size payload (body).

### 2.1. General Packet Layout

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|         Message Type          |         Payload Size          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
~                            Payload                            ~
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

### 2.2. Packet Header

The header is 4 bytes long and contains the following fields:

*   **Message Type** (2 bytes, `uint16_t`): An unsigned integer that identifies the type of message contained in the payload. This determines how the payload should be interpreted. See the "Message Types" section for a complete list of types.
*   **Payload Size** (2 bytes, `uint16_t`): An unsigned integer that specifies the size of the payload in bytes. This can be 0.

---

## 3. Message Types (Opcodes) and Payload Structures

This section details the payload structure for each `Message Type`. The `Message Type` is defined in `shared/net/Protocol.hpp`.

### 3.1. Gameplay Messages (UDP)

#### `CS_PLAYER_MOVE` (1)
*   **Payload:** `PlayerMoveData`
*   **Description:** Sent by the client to inform the server of the player's new position and velocity.
*   **Structure:**
    ```c++
    struct PlayerMoveData {
        uint32_t player_id;
        float    position_x;
        float    position_y;
        float    velocity_x;
        float    velocity_y;
    };
    ```

#### `CS_PLAYER_SHOOT` (2)
*   **Payload:** `PlayerShootData`
*   **Description:** Sent by the client when the player fires a weapon.
*   **Structure:**
    ```c++
    struct PlayerShootData {
        uint32_t player_id;
        uint16_t weapon_type;
        float    position_x;
        float    position_y;
        float    direction_x;
        float    direction_y;
    };
    ```

#### `SC_ENTITY_SPAWN` (3)
*   **Payload:** `EntitySpawnData`
*   **Description:** Sent by the server to instruct clients to create a new entity in the world.
*   **Structure:**
    ```c++
    struct EntitySpawnData {
        uint32_t entity_id;
        uint16_t entity_type; // See EntityType namespace
        uint16_t sub_type;
        float    position_x;
        float    position_y;
        float    velocity_x;
        float    velocity_y;
    };
    ```

#### `SC_ENTITY_MOVE` (4)
*   **Payload:** `EntityMoveData`
*   **Description:** Sent by the server to update the position and velocity of an existing entity.
*   **Structure:**
    ```c++
    struct EntityMoveData {
        uint32_t entity_id;
        float    position_x;
        float    position_y;
        float    velocity_x;
        float    velocity_y;
        uint8_t  flags; // Bit 0: is_hit
    };
    ```

#### `SC_ENTITY_DESTROY` (5)
*   **Payload:** `EntityDestroyData`
*   **Description:** Sent by the server to instruct clients to remove an entity from the world.
*   **Structure:**
    ```c++
    struct EntityDestroyData {
        uint32_t entity_id;
        uint8_t  reason; // See DestroyReason namespace
    };
    ```

#### `SC_GAME_STATE` (6)
*   **Payload:** `GameStateData`
*   **Description:** Sent periodically by the server to synchronize game state information with clients.
*   **Structure:**
    ```c++
    struct GameStateData {
        uint32_t game_time;
        uint16_t wave_number;
        uint16_t enemies_remaining;
        uint32_t score;
        uint8_t  game_state; // See GameState namespace
        uint8_t  lives;
        uint8_t  padding[2];
    };
    ```

#### `MSG_PING` (7) / `MSG_PONG` (8)
*   **Payload:** `PingPongData`
*   **Description:** Used for measuring network latency (RTT). A `MSG_PING` is sent, and the recipient replies with a `MSG_PONG` containing the original timestamp.
*   **Structure:**
    ```c++
    struct PingPongData {
        uint64_t timestamp; // Microseconds since epoch
    };
    ```

### 3.2. Lobby & Management Messages (TCP)

#### `CS_JOIN_SESSION` (10)
*   **Payload:** `PlayerJoinData`
*   **Description:** Sent by a client to join a game session.
*   **Structure:**
    ```c++
    struct PlayerJoinData {
        uint32_t session_id;
        uint32_t player_id;
        char     player_name[17];
    };
    ```

#### `CS_LEAVE_SESSION` (11)
*   **Payload:** `PlayerLeaveData`
*   **Description:** Sent by a client to leave the current game session.
*   **Structure:**
    ```c++
    struct PlayerLeaveData {
        uint32_t player_id;
    };
    ```

#### `CS_CHAT_MESSAGE` (12) / `SC_CHAT_MESSAGE` (13)
*   **Payload:** `ChatMessageData`
*   **Description:** Used to send and broadcast chat messages in the lobby.
*   **Structure:**
    ```c++
    struct ChatMessageData {
        uint32_t player_id;
        char     player_name[17];
        char     message[128];
    };
    ```

#### `SC_GAME_START` (14)
*   **Payload:** `GameStartData`
*   **Description:** Sent by the server to all clients in a lobby to signal the start of the game.
*   **Structure:**
    ```c++
    struct GameStartData {
        uint32_t session_id;
        uint16_t level_id;
        uint8_t  player_count;
        uint8_t  difficulty;
        uint32_t timestamp;
    };
    ```

#### `CS_CREATE_ROOM` (15)
*   **Payload:** `CreateRoomData`
*   **Description:** Client requests to create a new game room.
*   **Structure:**
    ```c++
    struct CreateRoomData {
        char    room_name[32];
        uint8_t max_players;
        uint8_t game_mode;
        uint8_t difficulty;
        uint8_t friendly_fire;
        uint8_t lives;
    };
    ```

#### `CS_JOIN_ROOM` (16)
*   **Payload:** `JoinRoomData`
*   **Description:** Client requests to join an existing game room by its session ID.
*   **Structure:**
    ```c++
    struct JoinRoomData {
        uint32_t session_id;
    };
    ```

#### `SC_ROOM_LIST` (17)
*   **Payload:** A series of `RoomInfoData` structures. The `payload_size` will be `N * sizeof(RoomInfoData)`.
*   **Description:** Server sends a list of available rooms to the client.
*   **Structure:**
    ```c++
    struct RoomInfoData {
        uint32_t session_id;
        uint8_t  player_count;
        uint8_t  max_players;
        uint8_t  status; // 0=lobby, 1=playing
        char     room_name[32];
    };
    ```

#### `SC_LOBBY_UPDATE` (18)
*   **Payload:** `LobbyUpdateData`
*   **Description:** Server sends an update about the lobby state to a client (e.g., when a player joins/leaves).
*   **Structure:**
    ```c++
    struct LobbyUpdateData {
        int8_t playerCount;
        int8_t yourPlayerId;
    };
    ```

---

## 4. Enumerations

### 4.1. `EntityType`
Used in `EntitySpawnData`.
```c++
namespace EntityType {
    constexpr uint16_t ENEMY = 0;
    constexpr uint16_t PROJECTILE = 1;
    constexpr uint16_t POWERUP = 2;
    constexpr uint16_t OBSTACLE = 3;
    constexpr uint16_t PLAYER = 4;
}
```

### 4.2. `DestroyReason`
Used in `EntityDestroyData`.
```c++
namespace DestroyReason {
    constexpr uint8_t TIMEOUT = 0;
    constexpr uint8_t COLLISION = 1;
    constexpr uint8_t KILLED = 2;
    constexpr uint8_t COLLECTED = 3;
    constexpr uint8_t OUT_OF_BOUNDS = 4;
}
```

### 4.3. `GameState`
Used in `GameStateData`.
```c++
namespace GameState {
    constexpr uint8_t LOBBY = 0;
    constexpr uint8_t PLAYING = 1;
    constexpr uint8_t PAUSED = 2;
    constexpr uint8_t GAME_OVER = 3;
    constexpr uint8_t BOSS_WARNING = 4;
    constexpr uint8_t STAGE_CLEARED = 5;
}
```