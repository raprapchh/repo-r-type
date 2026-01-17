#include "systems/MapGeneratorSystem.hpp"
#include "components/Position.hpp"
#include "components/HitBox.hpp"
#include "components/CollisionLayer.hpp"
#include "components/Drawable.hpp"
#include "components/Tag.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>

namespace rtype::ecs {

void MapGeneratorSystem::update(GameEngine::Registry& registry, float view_center_y) {
    if (!_initialized) {
        std::srand(std::time(nullptr));
        _initialized = true;
        _last_platform_y = 550.0f;
    }

    float generation_threshold = view_center_y - 800.0f;

    while (_last_platform_y > generation_threshold) {
        float gap_y = 80.0f + static_cast<float>(std::rand() % 70);
        float new_y = _last_platform_y - gap_y;

        float new_x = static_cast<float>(std::rand() % 700);

        auto platform = registry.createEntity();
        registry.addComponent<component::Position>(platform, new_x, new_y);
        registry.addComponent<component::HitBox>(platform, 100.0f, 20.0f);
        registry.addComponent<component::Collidable>(platform, component::CollisionLayer::Obstacle);
        registry.addComponent<component::Drawable>(platform, "green_platform", 0, 0, 100, 20);
        registry.addComponent<component::Tag>(platform, "Platform");

        if (std::rand() % 100 < 10) {
            float hole_x;
            bool valid = false;
            int attempts = 0;

            while (!valid && attempts < 10) {
                hole_x = static_cast<float>(std::rand() % 740);

                float platform_end = new_x + 100.0f;
                float hole_end = hole_x + 60.0f;
                float min_dist = 50.0f;

                bool overlap = (hole_x < platform_end + min_dist) && (hole_end + min_dist > new_x);

                if (!overlap) {
                    valid = true;
                }
                attempts++;
            }

            if (valid) {
                auto hole = registry.createEntity();
                registry.addComponent<component::Position>(hole, hole_x, new_y - 50.0f);
                registry.addComponent<component::HitBox>(hole, 60.0f, 60.0f);
                registry.addComponent<component::Drawable>(hole, "hole", 0, 0, 0, 0);
                registry.addComponent<component::Tag>(hole, "Hole");
            }
        }

        _last_platform_y = new_y;
    }

    float cleanup_threshold = view_center_y + 800.0f;
    auto view = registry.view<component::Position, component::Tag>();
    std::vector<GameEngine::entity_t> to_destroy;

    for (auto entity : view) {
        auto& pos = view.get<component::Position>(entity);
        if (pos.y > cleanup_threshold) {
            to_destroy.push_back(entity);
        }
    }

    for (auto entity : to_destroy) {
        registry.destroyEntity(entity);
    }
}

} // namespace rtype::ecs
