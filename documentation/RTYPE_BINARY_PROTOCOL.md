# RTYPE Binary Protocol Specification

**Status**: Active  
**Version**: 1.0  
**Date**: 2025  
**Transport**: UDP over ASIO  

## 1. Overview

This document specifies the RTYPE Binary Network Protocol (RBP), a lightweight UDP-based protocol designed for real-time multiplayer game communication. The protocol uses a fixed header structure followed by variable-length message bodies, enabling efficient serialization and deserialization of game state information.

### 1.1 Design Goals

- **Low Latency**: Minimize packet overhead and processing time
- **Simplicity**: Straightforward packet structure for easy implementation
- **Bandwidth Efficiency**: Binary encoding instead of text-based formats
- **Scalability**: Support multiple concurrent players and entities
- **Interoperability**: Both client and server use identical serialization rules

### 1.2 Transport Layer

- **Protocol**: UDP (User Datagram Protocol)
- **Port**: Server-configurable (default: 8080)
- **Reliability**: Best-effort (no built-in acknowledgment, see Section 5)
- **Ordering**: No guarantee (application layer must handle reordering)

---

## 2. Packet Structure

All network packets follow a consistent binary format:

```
┌─────────────────────────────────────┐
│     PACKET HEADER (4 bytes)         │
├──────────────────┬──────────────────┤
│  Message Type    │  Payload Size    │
│  (uint16_t)      │  (uint16_t)      │
│  2 bytes         │  2 bytes         │
├─────────────────────────────────────┤
│     PACKET BODY (Variable Length)   │
│  (size = Payload Size)              │
│  (0 to 1016 bytes max)              │
└─────────────────────────────────────┘
```

### 2.1 Packet Header Format

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `message_type` | uint16_t | 2 bytes | Identifies the packet type (OpCode). See Section 3. |
| `payload_size` | uint16_t | 2 bytes | Length of the packet body in bytes (0-1016). |

**Byte Order**: Little-endian (platform-dependent, handled by serializer)

**Maximum Packet Size**: 1024 bytes total (4-byte header + 1020-byte payload limit)

### 2.2 Packet Body Format

The body contains message-specific data serialized in binary format. Field ordering is strictly defined by the MessageSerializer implementation (see Section 4).

**Validation Rules**:
- Declared `payload_size` must exactly match actual body size
- `message_type` must be in range [1, 20]
- Empty bodies (payload_size = 0) are valid for certain message types
- All numeric types use host byte order (typically little-endian on x86)

---

## 3. Message Types (OpCodes)

Messages are identified by a 16-bit unsigned integer. The following types are defined:

| OpCode | Value | Direction | Purpose | Section |
|--------|-------|-----------|---------|---------|
| `PlayerJoin` | 1 | Bidirectional | Join/rejoin notification | 4.1 |
| `PlayerMove` | 2 | C→S, S→C | Player position/velocity update | 4.2 |
| `PlayerShoot` | 3 | C→S | Fire weapon action | 4.3 |
| `PlayerLeave` | 4 | S→C | Player disconnection | 4.4 |
| `EntitySpawn` | 5 | S→C | Create game entity | 4.5 |
| `EntityMove` | 6 | S→C | Update entity position | 4.6 |
| `EntityDestroy` | 7 | S→C | Remove entity | 4.7 |
| `GameStart` | 8 | S→C | Begin game session | 4.8 |
| `GameState` | 9 | S→C | Game state snapshot | 4.9 |
| `Ping` | 10 | C→S | Latency probe | 4.10 |
| `Pong` | 11 | S→C | Latency response | 4.11 |
| `MapResize` | 12 | C→S | Viewport/map resize notification | 4.12 |

**Message Flow**:
```
Client                              Server
  │                                   │
  ├──────── PlayerJoin(ID=0) ────────>│  (connection request)
  │                                   │
  │<───── PlayerJoin(ID=assigned) ────┤  (acknowledge + assign ID)
  │                                   │
  ├──────── PlayerMove() ────────────>│  (continuous updates)
  │                                   │
  │<───── PlayerMove() ────────────────┤  (other players' moves)
  │<───── EntitySpawn() ────────────────┤  (enemy spawned)
  │<───── GameState() ──────────────────┤  (periodic state update)
  │                                   │
  ├──────── PlayerShoot() ───────────>│  (fire action)
  │<───── EntityDestroy() ─────────────┤  (enemy defeated)
  │                                   │
```

---

## 4. Message Specifications

### 4.1 PlayerJoin (OpCode: 1)

**Direction**: Bidirectional (C→S request, S→C acknowledge)  
**Purpose**: Register or rejoin player, assign session ID

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
─────────────────────────────────────────────────────
0              | player_id | uint32_t | 4 | Assigned unique ID (0 if new)
```

**Client→Server**: Client sends with `player_id=0` to join.  
**Server→Client**: Server responds with assigned `player_id` (1-N).  

**Example (Hex)**:
```
Header: 01 00 04 00    (OpCode=1, Size=4)
Body:   00 00 00 00    (player_id=0, new connection)
```

---

### 4.2 PlayerMove (OpCode: 2)

**Direction**: Client→Server (then broadcast S→C)  
**Purpose**: Update player position and velocity

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | player_id | uint32_t | 4 | Player identifier
4              | position_x | float | 4 | X coordinate
8              | position_y | float | 4 | Y coordinate
12             | velocity_x | float | 4 | X velocity component
16             | velocity_y | float | 4 | Y velocity component
```

**Total Size**: 20 bytes

**Frequency**: High frequency (typically 30-60 Hz per client)  
**Ordering**: Server may reorder; clients use timestamps when available.

**Example (Hex)**:
```
Header: 02 00 14 00    (OpCode=2, Size=20)
Body:   01 00 00 00    (player_id=1)
        00 00 48 42    (position_x=50.0)
        00 00 20 42    (position_y=40.0)
        00 00 00 00    (velocity_x=0.0)
        00 00 80 bf    (velocity_y=-1.0)
```

---

### 4.3 PlayerShoot (OpCode: 3)

**Direction**: Client→Server  
**Purpose**: Initiate weapon fire from player

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | player_id | uint32_t | 4 | Shooting player ID
4              | weapon_type | uint16_t | 2 | Weapon enum
6              | position_x | float | 4 | Projectile spawn X
10             | position_y | float | 4 | Projectile spawn Y
14             | direction_x | float | 4 | Direction vector X
18             | direction_y | float | 4 | Direction vector Y
```

**Total Size**: 22 bytes

**Weapon Types** (application-defined):
- `0`: Standard laser
- `1`: Spread shot
- `2`: Homing missile
- (extended by game config)

**Direction**: Unit vector; should be normalized for consistent projectile speed.

---

### 4.4 PlayerLeave (OpCode: 4)

**Direction**: Server→Client  
**Purpose**: Notify all clients that a player has disconnected

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | player_id | uint32_t | 4 | Departed player ID
```

**Total Size**: 4 bytes

---

### 4.5 EntitySpawn (OpCode: 5)

**Direction**: Server→Client  
**Purpose**: Create a dynamic game entity (enemy, obstacle, powerup)

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | entity_id | uint32_t | 4 | Unique entity ID
4              | entity_type | uint16_t | 2 | Type classifier
6              | sub_type | uint16_t | 2 | Subtype variant
8              | position_x | float | 4 | Initial X
12             | position_y | float | 4 | Initial Y
16             | velocity_x | float | 4 | Initial velocity X
20             | velocity_y | float | 4 | Initial velocity Y
```

**Total Size**: 24 bytes

**Entity Types**:
```c
ENEMY      = 0    // Hostile unit
PROJECTILE = 1    // Bullet, missile, etc.
POWERUP    = 2    // Bonus item
OBSTACLE   = 3    // Static/dynamic hazard
PLAYER     = 4    // Networked player (for sync)
```

---

### 4.6 EntityMove (OpCode: 6)

**Direction**: Server→Client  
**Purpose**: Update entity position and velocity (non-player entities)

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | entity_id | uint32_t | 4 | Target entity
4              | position_x | float | 4 | Updated X
8              | position_y | float | 4 | Updated Y
12             | velocity_x | float | 4 | Updated velocity X
16             | velocity_y | float | 4 | Updated velocity Y
```

**Total Size**: 20 bytes

---

### 4.7 EntityDestroy (OpCode: 7)

**Direction**: Server→Client  
**Purpose**: Remove entity from game world

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | entity_id | uint32_t | 4 | Entity to destroy
4              | reason | uint8_t | 1 | Destruction cause
```

**Total Size**: 5 bytes

**Destruction Reasons**:
```c
TIMEOUT      = 0    // Life expired
COLLISION    = 1    // Hit detection triggered
KILLED       = 2    // Destroyed by player action
COLLECTED    = 3    // Consumed (powerup)
OUT_OF_BOUNDS = 4   // Left playable area
```

---

### 4.8 GameStart (OpCode: 8)

**Direction**: Server→Client  
**Purpose**: Signal start of game session; includes level/difficulty metadata

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | session_id | uint32_t | 4 | Unique session token
4              | level_id | uint16_t | 2 | Level/stage number
6              | player_count | uint8_t | 1 | Total players in game
7              | difficulty | uint8_t | 1 | Difficulty setting
8              | timestamp | uint32_t | 4 | Server timestamp (Unix seconds)
```

**Total Size**: 12 bytes

**Difficulty Levels**:
- `0`: Easy
- `1`: Normal
- `2`: Hard
- `3`: Extreme

---

### 4.9 GameState (OpCode: 9)

**Direction**: Server→Client  
**Purpose**: Periodic game state snapshot for synchronization

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | game_time | uint32_t | 4 | Elapsed time (seconds)
4              | wave_number | uint16_t | 2 | Current enemy wave
6              | enemies_remaining | uint16_t | 2 | Active enemy count
8              | score | uint32_t | 4 | Current score
12             | game_state | uint8_t | 1 | Game phase
13             | lives | uint8_t | 1 | Player lives remaining
14             | padding[0] | uint8_t | 1 | (reserved)
15             | padding[1] | uint8_t | 1 | (reserved)
```

**Total Size**: 16 bytes

**Game States**:
```c
LOBBY       = 0    // Waiting for players
PLAYING     = 1    // Active gameplay
PAUSED      = 2    // Paused
GAME_OVER   = 3    // Game ended
```

**Frequency**: Server sends every 50-100 ms (10-20 Hz) for synchronization.

---

### 4.10 Ping (OpCode: 10)

**Direction**: Client→Server  
**Purpose**: Measure network latency; client initiates

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | timestamp | uint64_t | 8 | Client's high-resolution timestamp
```

**Total Size**: 8 bytes

**Timestamp Format**: Milliseconds since epoch (recommended) or custom monotonic clock.

---

### 4.11 Pong (OpCode: 11)

**Direction**: Server→Client  
**Purpose**: Response to Ping; echoes client's timestamp

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | timestamp | uint64_t | 8 | Echo of Ping timestamp
```

**Total Size**: 8 bytes

**Latency Calculation**: `RTT = (receive_time - timestamp) * 2`

---

### 4.12 MapResize (OpCode: 12)

**Direction**: Client→Server  
**Purpose**: Notify server of viewport/window size changes

**Body Structure**:
```
Offset (bytes) | Field | Type | Size | Description
──────────────────────────────────────────────────────
0              | width | float | 4 | New viewport width
4              | height | float | 4 | New viewport height
```

**Total Size**: 8 bytes

**Use Case**: Allows server to adjust entity spawn regions and culling based on client viewport.

---

## 5. Reliability and Packet Loss Handling

### 5.1 Protocol Characteristics

The RTYPE Binary Protocol operates over UDP, which provides **no inherent reliability guarantees**. This design choice prioritizes low latency over message delivery certainty, suitable for real-time games where stale data becomes irrelevant quickly.

**UDP Properties**:
- No connection handshake
- No automatic retransmission
- No packet ordering guarantees
- Susceptible to loss, duplication, and reordering

### 5.2 Packet Loss Strategy

**Best-Effort Delivery**:
- Each packet is transmitted once without explicit acknowledgment
- Lost packets are not retransmitted
- Application logic must tolerate occasional missing updates

**Graceful Degradation**:
- Player movement: Extrapolate from last known velocity
- Entity updates: Use existing state if update missed
- Critical events (spawn/destroy): Re-send via periodic sync messages

### 5.3 Reliability Mechanisms

#### 5.3.1 State Synchronization

Frequent GameState updates (4.9) ensure periodic resync without requiring acknowledgment:
```
Server sends GameState → Client receives → Client state updated
                     ↓ (packet lost)
Client uses previous state → Server sends GameState again
```

**Recommended Interval**: 50-100 ms

#### 5.3.2 Sequence-Based Validation

The current protocol uses implicit sequencing via timestamp and entity/player IDs:
- **PlayerMove**: Identified by `player_id`; newer updates supersede older ones
- **EntityMove**: Identified by `entity_id`; spatial interpolation bridges gaps
- **EntitySpawn/Destroy**: Server broadcasts to all clients; redundant updates are idempotent

#### 5.3.3 Duplicate Handling

**Idempotent Operations**:
- **Position Updates** (PlayerMove, EntityMove): Last-write-wins
- **Entity Spawn**: Can be safely re-sent; duplicate creations are rejected by ID
- **Entity Destroy**: Destroying non-existent entity is benign

**Non-Idempotent Operations**:
- **Ping/Pong**: Duplicates inflate latency estimates; use timestamp to identify
- **PlayerShoot**: Duplicates create phantom projectiles (mitigated by validation)

#### 5.3.4 Validation and Error Detection

**Packet Validation** (ProtocolAdapter::validate):
```cpp
- Minimum size: sizeof(PacketHeader) bytes
- Total size: header.payload_size + header.size
- OpCode range: [1, 20]
- Payload size: ≤ 1016 bytes
```

**Mismatch Handling**:
```
if (header.payload_size != actual_body.size()) {
    discard packet
}
```

Prevents interpretation of malformed or corrupted packets.

### 5.4 Application-Level Resilience

#### 5.4.1 Extrapolation

For high-frequency updates (PlayerMove at 60 Hz), single missed packets have minimal impact:
```
Packet arrives at T=0:   pos=(100,100), vel=(-1, 0)
Packet missed at T=16ms
Packet arrives at T=32ms: pos=(98,100), vel=(-1, 0)

Client extrapolates: expected_pos ≈ (98, 100) [close to actual]
```

#### 5.4.2 Authoritative Server Model

Server maintains single source of truth:
- Server simulates authoritative physics
- Client submits actions (PlayerMove, PlayerShoot)
- Server broadcasts results to all clients

**Consequence**: Even if a client loses its own update, server re-sends via GameState/EntityMove.

#### 5.4.3 Timeout-Based Recovery

```
if (no_player_move_from_player_X for > 1 second) {
    assume disconnection
    broadcast PlayerLeave to other clients
}
```

Prevents "zombie" players in stale games.

### 5.5 Practical Performance

**Expected Loss Rate**: < 1% over LAN; 1-5% over internet  
**Impact**: Barely perceptible to players (1-2 missed updates per second at 30 Hz)  
**Workaround**: Increase GameState frequency if loss > 5%

**Example Configuration**:
```json
{
  "network": {
    "player_move_interval_ms": 16,    // 60 Hz client→server
    "game_state_interval_ms": 50,     // 20 Hz server→all
    "entity_move_interval_ms": 100,   // 10 Hz for non-critical entities
    "ping_interval_ms": 1000          // 1 Hz latency probe
  }
}
```

---

## 6. Serialization Details

### 6.1 Data Type Encoding

All data types are serialized in **binary format** (not JSON):

| Type | Encoding | Size | Example |
|------|----------|------|---------|
| `uint8_t` | Raw byte | 1 | `0x42` → 66 |
| `uint16_t` | Little-endian | 2 | `0x0102` → [0x02, 0x01] |
| `uint32_t` | Little-endian | 4 | `0x01020304` → [0x04, 0x03, 0x02, 0x01] |
| `uint64_t` | Little-endian | 8 | — |
| `float` | IEEE 754 | 4 | `3.14` → [0x4B, 0x0F, 0x49, 0x40] |

### 6.2 Field Ordering

Field ordering is strictly enforced in MessageSerializer:

**PlayerMove Example** (OpCode 2):
```cpp
serializer.write(data.player_id);      // uint32_t
serializer.write(data.position_x);     // float
serializer.write(data.position_y);     // float
serializer.write(data.velocity_x);     // float
serializer.write(data.velocity_y);     // float
```

**Deserialization must reverse this order**:
```cpp
uint32_t player_id = deserializer.read<uint32_t>();
float position_x = deserializer.read<float>();
float position_y = deserializer.read<float>();
// ... etc
```

### 6.3 Alignment

No padding is inserted between fields; serialization is **tightly packed**:
```
Offset 0:  player_id (4 bytes)     [0-3]
Offset 4:  position_x (4 bytes)    [4-7]
Offset 8:  position_y (4 bytes)    [8-11]
Offset 12: velocity_x (4 bytes)    [12-15]
Offset 16: velocity_y (4 bytes)    [16-19]
Total: 20 bytes (no gaps)
```

### 6.4 Serializer/Deserializer API

**Serialization**:
```cpp
rtype::net::Serializer serializer;
serializer.write(player_id);
serializer.write(position_x);
std::vector<uint8_t> data = serializer.get_data();
rtype::net::Packet packet(MessageType::PlayerMove, data);
```

**Deserialization**:
```cpp
rtype::net::Deserializer deserializer(packet.body);
uint32_t player_id = deserializer.read<uint32_t>();
float position_x = deserializer.read<float>();
```

---

## 7. Configuration Integration

### 7.1 Configuration File (JSON)

Game configuration is loaded from `config/game.json` and defines high-level parameters. However, **network packets are always binary**, never JSON.

**Example config.json**:
```json
{
  "network": {
    "server_host": "127.0.0.1",
    "server_port": 8080,
    "max_players": 4,
    "max_entities": 256,
    "player_move_freq_hz": 60,
    "game_state_freq_hz": 20,
    "max_latency_ms": 500
  },
  "game": {
    "max_levels": 10,
    "difficulty_levels": ["Easy", "Normal", "Hard", "Extreme"]
  }
}
```

### 7.2 Binary Protocol Mapping

Configuration values map to packet frequency and timeout logic:

| Config Key | Uses | Calculation |
|-----------|------|-------------|
| `player_move_freq_hz` | Client sends PlayerMove | interval = 1000 / freq_hz |
| `game_state_freq_hz` | Server sends GameState | interval = 1000 / freq_hz |
| `max_latency_ms` | Timeout detection | if (no update for 2× max_latency) → disconnect |

**Example**:
```
player_move_freq_hz = 60  →  Client sends PlayerMove every 16.67 ms
game_state_freq_hz = 20  →  Server sends GameState every 50 ms
```

---

## 8. Example Message Traces

### 8.1 Full Game Session Flow

```
Timeline   Direction   Message              Payload
─────────────────────────────────────────────────────
T=0ms      C→S         PlayerJoin           player_id=0 (request)
T=5ms      S→C         PlayerJoin           player_id=1 (assign)
T=10ms     C→S         MapResize            width=800, height=600
T=10ms     S→C         GameStart            session_id=42, level_id=1
T=50ms     S→C         GameState            LOBBY→PLAYING
T=50ms     S→C         EntitySpawn          entity_id=1, type=ENEMY
T=66ms     C→S         PlayerMove           player_id=1, vel=(-1, 0)
T=83ms     C→S         PlayerShoot          player_id=1, pos=(100,100)
T=100ms    S→C         EntityMove           entity_id=1, pos=(200,100)
T=100ms    S→C         GameState            wave=1, enemies=3, score=50
T=150ms    S→C         EntityDestroy        entity_id=1, reason=KILLED
T=200ms    C→S         Ping                 timestamp=200000
T=205ms    S→C         Pong                 timestamp=200000 [RTT≈5ms]
T=500ms    S→C         GameState            wave=2, enemies=5, score=200
T=1000ms   S→C         PlayerLeave          player_id=2 (timeout)
T=2000ms   S→C         GameState            PLAYING→GAME_OVER, score=5000
```

---

## 9. Error Handling

### 9.1 Validation Errors

**Invalid Packet Rejected**:
```
Received packet with payload_size=100 but body=50 bytes
→ Discard packet
→ Log error: "Malformed packet received"
```

**Invalid OpCode**:
```
Received OpCode=99 (undefined)
→ Discard packet
→ Log warning: "Unknown message type: 99"
```

### 9.2 Deserialization Errors

**Insufficient Data**:
```cpp
// Expecting 20 bytes for PlayerMove, but only 10 received
try {
    auto data = serializer.deserialize_player_move(packet);
} catch (const std::exception& e) {
    // "Deserializer: not enough data"
    // Discard packet, continue
}
```

### 9.3 Network Errors

**Send Failure**:
```cpp
udp_client_->send(packet_data);
// ASIO error_code reports failure
// Application retries or logs (UDP doesn't require ACK)
```

**Receive Timeout**:
```cpp
// No packets received for N seconds
// Trigger timeout logic (PlayerLeave)
```

---

## 10. Security Considerations

### 10.1 No Authentication

Current protocol includes **no built-in authentication**. Assumes private network or VPN.

**Risks**:
- Spoofed packets from untrusted clients
- Man-in-the-middle position updates

**Mitigation** (recommended):
- Validate server address before connecting
- Implement token-based auth in PlayerJoin payload (future version)

### 10.2 No Encryption

Packets are sent in plaintext. Network sniffing can expose:
- Player positions and actions
- Server state snapshots

**Mitigation** (recommended):
- Run game server on private LAN only
- Use VPN for internet play
- Implement TLS over UDP (e.g., DTLS) in future

### 10.3 No Checksum/Integrity

No CRC or HMAC included; corruption is detected by size mismatch only.

**Mitigation**:
- Most network stacks include IP/UDP checksums
- Application-level validation (ProtocolAdapter::validate) catches many errors

---

## 11. Appendix A: Quick Reference

### OpCode Summary

```c
#define OPCODE_PLAYER_JOIN     1
#define OPCODE_PLAYER_MOVE     2
#define OPCODE_PLAYER_SHOOT    3
#define OPCODE_PLAYER_LEAVE    4
#define OPCODE_ENTITY_SPAWN    5
#define OPCODE_ENTITY_MOVE     6
#define OPCODE_ENTITY_DESTROY  7
#define OPCODE_GAME_START      8
#define OPCODE_GAME_STATE      9
#define OPCODE_PING            10
#define OPCODE_PONG            11
#define OPCODE_MAP_RESIZE      12
```

### Common Packet Sizes

| Message | Header | Body | Total |
|---------|--------|------|-------|
| PlayerJoin | 4 | 4 | 8 |
| PlayerMove | 4 | 20 | 24 |
| PlayerShoot | 4 | 22 | 26 |
| EntitySpawn | 4 | 24 | 28 |
| EntityMove | 4 | 20 | 24 |
| EntityDestroy | 4 | 5 | 9 |
| GameState | 4 | 16 | 20 |

---

## 12. Appendix B: Future Enhancements

1. **Sequence Numbers**: Add uint32_t sequence to header for reordering detection
2. **Compression**: Implement optional zlib compression for large EntitySpawn batches
3. **Encryption**: Add optional AES-128-GCM mode
4. **Handshake**: Implement TCP-like 3-way handshake for reliability setup
5. **Fragmentation**: Support packets > 1024 bytes via multi-part protocol

---

## 13. References

- **ASIO Documentation**: https://think-async.com/Asio/
- **UDP RFC**: RFC 768 (https://tools.ietf.org/html/rfc768)
- **IEEE 754 Floating Point**: Standard for float serialization
- **Protocol Design Best Practices**: Game Networking Concepts by Glenn Fiedler

---

**Document Version**: 1.0
**Author(s)**: Swann Grandin
**Last Updated**: 2025-12  
**Status**: Active (Production)  
**Maintainer**: RTYPE Development Team
