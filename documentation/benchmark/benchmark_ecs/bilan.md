# ECS Benchmark for R-Type

## Context

The R-Type subject (page 13) recommends using the **Entity-Component-System** pattern for the game engine. This benchmark compares two approaches: EnTT (optimized library) vs a simple custom implementation.

## Comparison

### EnTT
- **Advantages**: Header-only, very fast (cache-friendly), easy to integrate with CMake
- **Disadvantages**: Moderate learning curve

### Custom Simple ECS
- **Advantages**: Total control, educational, no dependencies
- **Disadvantages**: Limited performance, more maintenance

## Results (10,000 entities, 100 iterations)

```
EnTT:          ~10 ms
Custom Simple: ~50 ms
```

EnTT is **~5x faster** than the simple implementation.

## Recommendation

**✅ EnTT** is recommended for R-Type because:
- Performance essential for real-time networked game (60 FPS)
- Contiguous component storage → facilitates network serialization
- Header-only → easy integration (Conan, vcpkg, CMake)
- Enables required decoupling (Rendering, Physics, Network systems)

## Installation

```bash
# Conan
conan install entt/3.13.0

# vcpkg
vcpkg install entt
```

## R-Type Usage Example

```cpp
entt::registry registry;

// Create an entity
auto enemy = registry.create();
registry.emplace<Position>(enemy, 800.0f, 300.0f);
registry.emplace<Velocity>(enemy, -2.0f, 0.0f);
registry.emplace<Sprite>(enemy, enemyTexture);

// Movement system
auto view = registry.view<Position, Velocity>();
for(auto entity : view) {
    auto [pos, vel] = view.get<Position, Velocity>(entity);
    pos.x += vel.dx;
    pos.y += vel.dy;
}
```

## How to Test

```bash
# Compile with EnTT
g++ -std=c++17 -O3 -I/path/to/entt/include bench_entt.cpp -o bench_entt

# Compile custom simple (no dependencies)
g++ -std=c++17 -O3 bench_custom_simple.cpp -o bench_custom_simple

# Execute
./bench_entt
./bench_custom_simple
```

## References

- [EnTT GitHub](https://github.com/skypjack/entt)
- [Game Programming Patterns - Component](https://gameprogrammingpatterns.com/component.html)
