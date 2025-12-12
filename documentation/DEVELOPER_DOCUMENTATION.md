# R-Type - Developer Documentation

Complete documentation to understand, develop, and contribute to the R-Type project.

**Version:** 1.0 | **Last Updated:** December 2025

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Quick Start](#2-quick-start)
3. [Project Architecture](#3-project-architecture)
4. [Entity Component System (ECS)](#4-entity-component-system-ecs)
5. [Networking System](#5-networking-system)
6. [Rendering Systems](#6-rendering-systems)
7. [Development Guide](#7-development-guide)
8. [Testing & Quality](#8-testing--quality)
9. [Contributing](#9-contributing)

---

## 1. Introduction

### Overview

R-Type is a recreation of the classic arcade game developed as part of the **Advanced C++ Knowledge (B-CPP-500)** module at Epitech. The project features:

- **ECS Architecture** based on EnTT for high performance
- **Networked Multiplayer** via UDP with authoritative server model
- **SFML Rendering** for graphics and audio
- **C++20** leveraging modern language features

### Prerequisites

| Tool | Minimum Version | Purpose |
|------|----------------|---------|
| C++ Compiler | C++20 | g++ 10+, clang 12+, MSVC 19.28+ |
| CMake | 3.20+ | Build system |
| Git | 2.0+ | Version control |

**Dependencies** (automatically managed by vcpkg):
- SFML 2.5+ (graphics, audio)
- EnTT (Entity Component System)
- ASIO (asynchronous networking)
- Catch2 (unit testing)

---

## 2. Quick Start

### Installation and Build

```bash
# Clone the project
git clone git@github.com:EpitechPGE3-2025/G-CPP-500-PAR-5-2-rtype-3.git
cd G-CPP-500-PAR-5-2-rtype-3

# Build (first build: 5-10 min to download dependencies)
./build.sh

# Clean
./clean.sh
```

### Running the Game

```bash
# Terminal 1: Start the server
./r-type_server 4242

# Terminal 2: Start the client
./r-type_client 127.0.0.1 4242
```

### Controls

| Action | Keys | Description |
|--------|------|-------------|
| Movement | WASD / Arrows | Move the spaceship |
| Shoot | Space | Fire a projectile |

---

## 3. Project Architecture

### Authoritative Client-Server Model

```
┌─────────────┐                    ┌─────────────┐
│   Client    │                    │   Server    │
│             │──── PlayerMove ───▶│             │
│ (Prediction)│                    │ (Validator) │
│             │◀─── GameState ─────│             │
│  (Render)   │                    │   (Logic)   │
└─────────────┘                    └─────────────┘
```

**Server** (Authoritative):
- Executes all game logic (collisions, spawning, scoring)
- Validates client inputs
- Broadcasts game state to all clients (20 Hz)
- Manages connections/disconnections

**Client** (Display):
- Captures player inputs
- Applies local prediction (responsiveness)
- Interpolates entity positions (smoothness)
- Renders game state received from server

### Directory Structure

```
r-type/
├── client/              # Client application
│   ├── include/        # Client headers (Client.hpp, Renderer.hpp, States.hpp)
│   ├── src/            # Implementations
│   ├── sprites/        # Textures (.png, .gif)
│   ├── fonts/          # Fonts (.otf)
│   └── assets/         # Sounds (.wav)
│
├── server/              # Server application
│   ├── include/        # Server headers (Server.hpp, Instance.hpp, UdpServer.hpp)
│   └── src/            # Implementations
│
├── ecs/                 # ECS library
│   ├── include/
│   │   ├── Registry.hpp         # Entity manager (EnTT wrapper)
│   │   ├── components/          # 20 components (Position, Velocity, Health...)
│   │   └── systems/             # 12 systems (Movement, Collision, Weapon...)
│   └── src/            # Implementations
│
├── shared/              # Common client/server code
│   ├── net/            # Network protocol (Protocol.hpp, MessageSerializer.hpp)
│   ├── interfaces/     # Abstract interfaces (ISystem, IEntityRegistry)
│   └── utils/          # Utilities (Logger, GameConstants)
│
├── tests/               # Unit tests (Catch2)
├── documentation/       # Documentation (you are here!)
└── external/            # Third-party libraries (EnTT, ASIO)
```

### Game Loops

**Server (60 Hz):**
1. Receive client inputs
2. Update game logic (ECS systems)
3. Broadcast game state (20 Hz)
4. Sleep 16.67ms (maintain 60 FPS)

**Client (60 FPS):**
1. Handle user inputs
2. Send movements to server
3. Receive server state
4. Interpolate entities
5. Render frame

---

## 4. Entity Component System (ECS)

### ECS Principles

**Entity:** Unique identifier (uint32_t)
**Component:** Pure data structure (no logic)
**System:** Logic operating on components

### Registry API

```cpp
Registry registry;

// Create an entity
auto entity = registry.spawn_entity();

// Add components
registry.add_component<Position>(entity, Position{100.0f, 200.0f});
registry.add_component<Velocity>(entity, Velocity{5.0f, 0.0f});

// Get a component
auto &pos = registry.get_component<Position>(entity);

// Destroy an entity
registry.kill_entity(entity);
```

### Main Components (20 total)

#### Transform & Physics
- **Position** (`x, y`) - 2D position of the entity
- **Velocity** (`vx, vy`) - Movement vector in pixels/second
- **HitBox** (`width, height`) - AABB collision box
- **CollisionLayer** (`layer`) - Collision filter (Player, Enemy, Projectile...)

#### Combat & Gameplay
- **Health** (`hp, max_hp`) - Health points
- **Weapon** (`fireRate, damage, projectileSpeed...`) - Weapon properties
- **Projectile** (`damage, owner_id, lifetime`) - Projectile data
- **Score** (`points`) - Player score
- **Lives** (`count`) - Remaining lives
- **InvincibilityTimer** (`remaining_time`) - Temporary invincibility

#### Rendering & Visual
- **Drawable** (`texture_name, rect, frames...`) - Sprite and animation
- **Explosion** (`timer, radius`) - Explosion effect

#### Networking
- **NetworkId** (`network_id`) - Network identifier of the entity
- **NetworkInterpolation** (`last_x, target_x, alpha...`) - Network interpolation

#### Utilities
- **Tag** (`tag`) - String classification (e.g., "Player", "Boss")
- **Controllable** - Marker for controllable entities
- **MapBounds** (`min_x, max_x, min_y, max_y`) - Movement boundaries
- **EnemySpawner** (`spawnInterval, timeSinceLastSpawn`) - Enemy spawner
- **Sound** (`sound_name, playing`) - Sound trigger

### Main Systems (12 total)

#### Server (Authoritative Logic)
1. **MovementSystem** - Updates Position based on Velocity
2. **CollisionSystem** - AABB detection with layer filtering
3. **WeaponSystem** - Manages cooldowns and spawns projectiles
4. **ProjectileSystem** - Updates projectile lifetime
5. **SpawnSystem** - Generates enemy waves
6. **MobSystem** - Enemy artificial intelligence
7. **BoundarySystem** - Enforces map boundaries
8. **LivesSystem** - Manages lives and respawning
9. **ScoreSystem** - Calculates and updates score

#### Client (Display)
10. **RenderSystem** - SFML sprite rendering
11. **AudioSystem** - Sound effect playback
12. **InputSystem** - Keyboard/mouse capture

### System Execution Order

Order is critical as some systems depend on others:

```cpp
// Server
MovementSystem       // 1. Update positions
CollisionSystem      // 2. Detect collisions (after movement)
WeaponSystem         // 3. Create projectiles
ProjectileSystem     // 4. Manage projectiles
SpawnSystem          // 5. Generate enemies
MobSystem            // 6. Enemy AI
BoundarySystem       // 7. Apply boundaries
LivesSystem          // 8. Manage lives
ScoreSystem          // 9. Calculate score
```

### Example: Creating a Player Entity

```cpp
auto player = registry.spawn_entity();
registry.add_component<Position>(player, Position{100.0f, 540.0f});
registry.add_component<Velocity>(player, Velocity{0.0f, 0.0f});
registry.add_component<Health>(player, Health{100, 100});
registry.add_component<HitBox>(player, HitBox{32.0f, 32.0f});
registry.add_component<Controllable>(player);
registry.add_component<Score>(player, Score{0});
registry.add_component<Lives>(player, Lives{3});
```

---

## 5. Networking System

### Binary UDP Protocol

**Why UDP?**
- Minimal latency (no retransmission)
- Acceptable for real-time games (packet loss OK)
- Frequent synchronization compensates for losses

### Packet Structure

```
┌────────────────────────────┐
│  HEADER (4 bytes)          │
├─────────────┬──────────────┤
│ Message Type│ Payload Size │
│ (uint16_t)  │ (uint16_t)   │
├─────────────┴──────────────┤
│  BODY (Variable)           │
└────────────────────────────┘
```

### Message Types (12 OpCodes)

| OpCode | Name | Direction | Frequency | Description |
|--------|------|-----------|-----------|-------------|
| 1 | PlayerJoin | ⇄ | Once | Connection/ID assignment |
| 2 | PlayerMove | Client→Server | 60 Hz | Position/velocity |
| 3 | PlayerShoot | Client→Server | On action | Weapon fire |
| 4 | PlayerLeave | ⇄ | Once | Disconnection |
| 5 | EntitySpawn | Server→Client | On event | Entity creation |
| 6 | EntityMove | Server→Client | 20 Hz | Entity update |
| 7 | EntityDestroy | Server→Client | On event | Entity destruction |
| 8 | GameStart | Server→Client | Once | Game start |
| 9 | GameState | Server→Client | 20 Hz | Full synchronization |
| 10 | Ping | ⇄ | 1 Hz | Latency measurement |
| 11 | Pong | ⇄ | 1 Hz | Latency response |
| 12 | MapResize | Client→Server | On resize | Viewport resize |

### Network Strategies

**Client-Side Prediction:**
- Apply input locally immediately
- Reduces perceived latency
- Server sends authoritative position that overrides prediction

**Network Interpolation:**
- Smooths movements between updates (20 Hz → 60 FPS)
- Uses `NetworkInterpolation` component
- Interpolates from `last_position` to `target_position`

**State Synchronization:**
- Server sends full state every 50ms (20 Hz)
- Self-correcting (lost packets compensated by next one)
- No delta compression (simplicity > optimization)

### Security

**Server Validation:**
- Never trust client inputs
- Validate positions (maximum possible distance)
- Check actions (weapon cooldown, resources)
- Rate limiting (limit messages/second)

---

## 6. Rendering Systems

### Rendering Pipeline (Client)

```cpp
void Renderer::render(Registry &registry) {
    window.clear(sf::Color::Black);

    renderBackground();           // 1. Background (parallax scrolling)
    renderEntities(registry);     // 2. Entities (Position + Drawable)
    renderUI(registry);           // 3. UI (score, lives)
    renderEffects(registry);      // 4. Effects (explosions)

    window.display();
}
```

### Animation System

Animated sprites use the `Drawable` component:
- `frames`: List of texture rectangles
- `current_frame`: Current frame index
- `frame_time`: Duration per frame (e.g., 0.1s = 10 FPS)
- A system updates `current_frame` based on elapsed time

### Texture Management

**Texture cache** to avoid reloading:
```cpp
std::unordered_map<std::string, sf::Texture> m_textureCache;
```

Textures are loaded once, reused for all sprites.

---

## 7. Development Guide

### Environment Setup

**Recommended IDEs:**
- **VSCode** + C/C++ extensions, CMake Tools
- **CLion** (built-in CMake support)
- **Vim/Neovim** + coc-clangd

### Code Style

**Standard:** Strict C++20

**Naming conventions:**
- Classes/Structs: `PascalCase`
- Functions: `camelCase`
- Variables: `snake_case`
- Constants: `UPPER_SNAKE_CASE`
- Private members: `m_` prefix

**Formatting:** clang-format (Google style)

```bash
# Format all files
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

### Commit Messages

**Conventional Commits** format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:** `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

**Examples:**
```
feat(ecs): add Explosion component for death animations
fix(network): resolve player ID desync in multiplayer
docs(api): update ECS system documentation
```

### Git Workflow

```bash
# Create a feature branch from dev
git checkout dev
git pull origin dev
git checkout -b feature/new-feature

# Make commits
git add .
git commit -m "feat(ecs): add new enemy type"

# Push and create PR to dev
git push origin feature/new-feature
```

**Branches:**
- `main`: Stable production
- `dev`: Feature integration
- `feature/<name>`: Feature development
- `fix/<name>`: Bug fixes

### Adding a New Component

1. Create `ecs/include/components/MyComponent.hpp`:
```cpp
#pragma once

namespace rtype::ecs::component {

struct MyComponent {
    float value;
    int counter;
};

} // namespace rtype::ecs::component
```

2. Use in code:
```cpp
#include "components/MyComponent.hpp"
registry.add_component<MyComponent>(entity, MyComponent{1.5f, 0});
```

### Adding a New System

1. Create `ecs/include/systems/MySystem.hpp`:
```cpp
#pragma once
#include "../../shared/interfaces/ecs/ISystem.hpp"

namespace rtype::ecs {

class MySystem : public ISystem {
public:
    void update(GameEngine::Registry &registry, double dt) override;
};

} // namespace rtype::ecs
```

2. Implement in `ecs/src/systems/MySystem.cpp`:
```cpp
#include "systems/MySystem.hpp"
#include "components/MyComponent.hpp"

void MySystem::update(GameEngine::Registry &registry, double dt) {
    auto view = registry.view<MyComponent>();
    for (auto entity : view) {
        auto &comp = view.get<MyComponent>(entity);
        // System logic
    }
}
```

3. Add to server/client:
```cpp
MySystem m_mySystem;

void update() {
    m_mySystem.update(m_registry, m_deltaTime);
}
```

### Debugging

**Server:**
```bash
gdb ./r-type_server
(gdb) run 4242
(gdb) break Server::update
```

**Client:**
```bash
gdb ./r-type_client
(gdb) run 127.0.0.1 4242
(gdb) break Renderer::render
```

**Network (Wireshark):**
```
udp.port == 4242
```

---

## 8. Testing & Quality

### Running Tests

```bash
# Build and run tests
./build.sh
./unit_tests

# Verbose mode
./unit_tests -s

# Specific test
./unit_tests "[MovementSystem]"
```

### Test Structure

Tests use **Catch2 v3**:

```cpp
#include <catch2/catch_test_macros.hpp>
#include "systems/MovementSystem.hpp"

TEST_CASE("MovementSystem updates position", "[MovementSystem]") {
    Registry registry;
    auto entity = registry.spawn_entity();
    registry.add_component<Position>(entity, Position{0.0f, 0.0f});
    registry.add_component<Velocity>(entity, Velocity{10.0f, 5.0f});

    MovementSystem system;
    system.update(registry, 1.0);

    auto &pos = registry.get_component<Position>(entity);
    REQUIRE(pos.x == 10.0f);
    REQUIRE(pos.y == 5.0f);
}
```

### CI/CD

**GitHub Actions** (`.github/workflows/ci.yml`):
1. Commit message validation
2. Code formatting check (clang-format)
3. Project compilation
4. Test execution

---

## 9. Contributing

### Pull Request Checklist

Before submitting a PR:

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] Code formatted (clang-format)
- [ ] Commit messages conform to convention
- [ ] No conflicts with target branch
- [ ] Tests added for new features
- [ ] Documentation updated if needed

### Code Review

**Reviewers check:**
- Logic correctness
- Performance implications
- Security considerations
- Code style compliance
- Test coverage

### Best Practices

**ECS:**
- Small, focused components
- Stateless systems
- Use views for efficient iteration

**Network:**
- Always validate client inputs
- Minimize packet size
- Reduce frequency of non-critical messages

**Performance:**
- Avoid allocations in game loop
- Use smart pointers (unique_ptr, shared_ptr)
- Batch entity destructions

---

## Additional Resources

### Internal Documentation
- [Detailed Network Protocol](./RTYPE_BINARY_PROTOCOL.md) - Complete protocol specification
- [Contribution Guide](./how_to_contribute.md) - How to contribute to the project

### External Resources
- [EnTT Documentation](https://github.com/skypjack/entt) - ECS library documentation
- [SFML Tutorials](https://www.sfml-dev.org/tutorials/) - SFML tutorials
- [ASIO Documentation](https://think-async.com/Asio/) - ASIO documentation
- [Game Networking](https://gafferongames.com/) - Game networking best practices

### Support

- **GitHub Issues:** Report bugs or request features
- **Documentation:** Check the `/documentation` folder
- **Code Comments:** Most complex logic has inline comments

---

## Team

**Epitech Paris - Class of 2025**

- Raphaël Chanliongco
- Hubert Touraine
- Jean-Baptiste Boshra
- Gabin Rudigoz
- Ylan Cuvier
- Swann Grandin

---
