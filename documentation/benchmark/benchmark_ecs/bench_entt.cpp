#include <iostream>
#include <chrono>
#include <entt/entt.hpp>

struct Position {
    float x, y;
};
struct Velocity {
    float dx, dy;
};

int main() {
    entt::registry registry;
    const int NUM_ENTITIES = 10000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_ENTITIES; ++i) {
        auto entity = registry.create();
        registry.emplace<Position>(entity, float(i), float(i));
        registry.emplace<Velocity>(entity, 1.0f, 1.0f);
    }

    for (int iter = 0; iter < 100; ++iter) {
        auto view = registry.view<Position, Velocity>();
        for (auto entity : view) {
            auto [pos, vel] = view.get<Position, Velocity>(entity);
            pos.x += vel.dx;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "EnTT: " << total << " ms\n";
    return 0;
}
