# RenderSystem Refactoring - Architecture Document

## Overview

The rendering logic has been successfully isolated from the core ECS engine, allowing the system to operate in both graphical and headless modes.

## Architecture

### 1. Abstract Rendering Interface

**File:** `shared/interfaces/rendering/IRenderer.hpp`

- Defines the `IRenderer` interface that all rendering backends must implement
- Provides a `RenderData` struct containing all necessary information to render an entity without SFML dependencies
- Methods:
  - `draw_sprite(const RenderData&)` - Draw a sprite
  - `clear()` - Clear the render target
  - `display()` - Swap/display the render target
  - `is_open()` - Check if rendering is available
  - `get_texture_size(...)` - Query texture dimensions

### 2. Refactored RenderSystem

**Files:** `ecs/include/systems/RenderSystem.hpp`, `ecs/src/systems/RenderSystem.cpp`

- Now headless-capable: operates without a renderer
- Accepts `std::shared_ptr<IRenderer>` instead of direct SFML references
- In headless mode (no renderer), `update()` is a no-op
- Handles all entity animation and sprite frame management
- Delegates rendering to the abstract IRenderer interface

**Key methods:**
```cpp
RenderSystem(std::shared_ptr<rtype::rendering::IRenderer> renderer = nullptr,
             rtype::client::AccessibilityManager* accessibility_mgr = nullptr);
void set_renderer(std::shared_ptr<rtype::rendering::IRenderer> renderer);
std::shared_ptr<rtype::rendering::IRenderer> get_renderer() const;
```

### 3. SFML Rendering Backend

**Files:** `client/include/SFMLRenderer.hpp`, `client/src/SFMLRenderer.cpp`

- Concrete implementation of `IRenderer` using SFML
- Encapsulates all SFML-specific logic:
  - Texture management
  - Sprite creation and rendering
  - Window operations
  - Texture atlas calculations (player_ships, explosions, etc.)

**Usage:**
```cpp
auto sfml_renderer = std::make_shared<rtype::rendering::SFMLRenderer>(
    *window, textures);
rtype::ecs::RenderSystem render_system(sfml_renderer, accessibility_mgr);
```

### 4. Null Renderer (Headless Mode)

**File:** `shared/interfaces/rendering/NullRenderer.hpp`

- No-operation implementation of `IRenderer`
- Used for headless operation (server simulations, automated testing)
- All methods are no-ops, allowing the ECS engine to run without any graphics

**Usage:**
```cpp
auto null_renderer = std::make_shared<rtype::rendering::NullRenderer>();
rtype::ecs::RenderSystem render_system(null_renderer);
// or simply:
rtype::ecs::RenderSystem render_system(nullptr);  // render() becomes no-op
```

## Benefits

1. **Headless Capability**: Run the entire ECS engine without any rendering
2. **Backend Flexibility**: Easy to add new rendering backends (OpenGL, Vulkan, etc.)
3. **Testability**: Test game logic without rendering overhead
4. **Separation of Concerns**: Rendering logic isolated from core game logic
5. **SFML Independence**: Core engine has zero SFML dependencies

## Usage Examples

### 1. Client with SFML Rendering

```cpp
#include "client/include/SFMLRenderer.hpp"
#include "ecs/include/systems/RenderSystem.hpp"

sf::RenderWindow window(sf::VideoMode(1280, 720), "R-Type");
std::unordered_map<std::string, sf::Texture> textures;
// ... load textures ...

auto renderer = std::make_shared<rtype::rendering::SFMLRenderer>(window, textures);
rtype::ecs::RenderSystem render_system(renderer, accessibility_mgr);

GameEngine::Registry registry;
// ... populate registry ...

render_system.update(registry, delta_time);
```

### 2. Server in Headless Mode

```cpp
#include "shared/interfaces/rendering/NullRenderer.hpp"
#include "ecs/include/systems/RenderSystem.hpp"

// Option 1: Explicitly use NullRenderer
auto null_renderer = std::make_shared<rtype::rendering::NullRenderer>();
rtype::ecs::RenderSystem render_system(null_renderer);

// Option 2: Use nullptr (RenderSystem::update() becomes no-op)
rtype::ecs::RenderSystem render_system(nullptr);

GameEngine::Registry registry;
// ... game logic runs without any rendering ...
render_system.update(registry, delta_time);  // Does nothing
```

### 3. Dynamic Renderer Switching

```cpp
rtype::ecs::RenderSystem render_system;  // No renderer initially

// Later, add SFML renderer
auto renderer = std::make_shared<rtype::rendering::SFMLRenderer>(window, textures);
render_system.set_renderer(renderer);

// Query current renderer
auto current = render_system.get_renderer();
```

## Implementation Details

### Animation Handling

The `RenderSystem` still handles all animation logic:
- Frame updates based on animation sequences
- State-based animation (walk, run, die, etc.)
- Sprite sheet coordinate calculations
- Explosion cleanup

This ensures game logic remains consistent whether rendered or not.

### Texture Rectangle Calculation

The `SFMLRenderer::calculate_texture_rect()` method handles:
- Multi-row sprite sheets (player_ships: 5x5 grid)
- Custom rectangle coordinates (explosions)
- Frame-based animation (default horizontal strips)
- Special cases (obstacle_1)

### Accessibility Support

The `RenderSystem` still applies accessibility color overlays through the `AccessibilityManager`, which is independent of rendering.

## Compilation

The system compiles successfully with all new files:
- ✅ `shared/interfaces/rendering/IRenderer.hpp`
- ✅ `shared/interfaces/rendering/NullRenderer.hpp`
- ✅ `client/include/SFMLRenderer.hpp`
- ✅ `client/src/SFMLRenderer.cpp`
- ✅ Updated `ecs/include/systems/RenderSystem.hpp`
- ✅ Updated `ecs/src/systems/RenderSystem.cpp`
- ✅ Updated `client/src/GameState.cpp`

## Testing Recommendations

1. **Headless Mode**: Start a RenderSystem with nullptr and verify game logic runs
2. **SFML Rendering**: Verify visual output matches previous implementation
3. **Performance**: Compare rendering performance between SFML and null renderers
4. **Switching**: Test dynamic renderer switching at runtime
5. **Edge Cases**: Test animation with/without renderer

## Future Enhancements

1. Add OpenGL-based renderer implementation
2. Add Vulkan-based renderer implementation
3. Create renderer factory for easier instantiation
4. Add renderer-specific optimizations
5. Implement renderer statistics/profiling
