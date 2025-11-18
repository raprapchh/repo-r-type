#include <iostream>
#include <chrono>
#include <vector>
#include <unordered_map>

struct Position { float x, y; };
struct Velocity { float dx, dy; };

class SimpleECS {
    struct Entity {
        Position* pos = nullptr;
        Velocity* vel = nullptr;
    };
    std::vector<Entity> entities;

public:
    void createEntity(float x, float y) {
        Entity e;
        e.pos = new Position{x, y};
        e.vel = new Velocity{1.0f, 1.0f};
        entities.push_back(e);
    }

    void update() {
        for (auto& e : entities) {
            if (e.pos && e.vel)
                e.pos->x += e.vel->dx;
        }
    }

    ~SimpleECS() {
        for (auto& e : entities) {
            delete e.pos;
            delete e.vel;
        }
    }
};

int main() {
    SimpleECS ecs;
    const int NUM_ENTITIES = 10000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_ENTITIES; ++i) {
        ecs.createEntity(float(i), float(i));
    }

    for (int iter = 0; iter < 100; ++iter) {
        ecs.update();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "Custom Simple: " << total << " ms\n";
    return 0;
}
