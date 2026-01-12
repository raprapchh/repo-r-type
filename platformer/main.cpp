#include <SFML/Graphics.hpp>

#include <memory>
#include <vector>

#include "../ecs/include/Registry.hpp"
#include "../ecs/include/components/Position.hpp"
#include "../ecs/include/components/Velocity.hpp"
#include "../ecs/include/components/Drawable.hpp"
#include "../ecs/include/components/Controllable.hpp"
#include "../ecs/include/components/HitBox.hpp"
#include "../ecs/include/components/CollisionLayer.hpp"
#include "../ecs/include/components/Tag.hpp"
#include "../ecs/include/components/Gravity.hpp"
#include "../ecs/include/components/Jump.hpp"
#include "../client/include/SFMLRenderer.hpp"
#include "../ecs/include/systems/RenderSystem.hpp"
#include "../ecs/include/systems/MovementSystem.hpp"
#include "../ecs/include/systems/PlatformerPhysicsSystem.hpp"

int main() {
    GameEngine::Registry registry;
    sf::RenderWindow window(sf::VideoMode(800, 600), "Platformer PoC");
    window.setFramerateLimit(60);

    auto renderer_impl =
        std::make_shared<rtype::rendering::SFMLRenderer>(window, *(new std::unordered_map<std::string, sf::Texture>()));
    auto render_system = std::make_shared<rtype::ecs::RenderSystem>(renderer_impl, nullptr);

    auto movement_system = std::make_shared<rtype::ecs::MovementSystem>();
    auto physics_system = std::make_shared<rtype::ecs::PlatformerPhysicsSystem>();

    auto player = registry.createEntity();
    registry.addComponent<rtype::ecs::component::Position>(player, 100.0f, 100.0f);
    registry.addComponent<rtype::ecs::component::Velocity>(player, 0.0f, 0.0f);
    registry.addComponent<rtype::ecs::component::HitBox>(player, 50.0f, 50.0f);
    registry.addComponent<rtype::ecs::component::Drawable>(player, "__RECTANGLE__", 100, 100, 50, 50);
    registry.addComponent<rtype::ecs::component::Controllable>(player);
    registry.addComponent<rtype::ecs::component::Gravity>(player);
    registry.addComponent<rtype::ecs::component::Jump>(player);
    registry.addComponent<rtype::ecs::component::Tag>(player, "Player");

    struct PlatformConfig {
        float x, y, width, height;
    };

    std::vector<PlatformConfig> level_layout = {{0.0f, 550.0f, 800.0f, 50.0f},   {200.0f, 450.0f, 100.0f, 20.0f},
                                                {400.0f, 350.0f, 100.0f, 20.0f}, {150.0f, 250.0f, 100.0f, 20.0f},
                                                {500.0f, 150.0f, 100.0f, 20.0f}, {300.0f, 50.0f, 100.0f, 20.0f},
                                                {100.0f, -50.0f, 100.0f, 20.0f}, {600.0f, -150.0f, 100.0f, 20.0f}};

    for (const auto& config : level_layout) {
        auto wall = registry.createEntity();
        registry.addComponent<rtype::ecs::component::Position>(wall, config.x, config.y);
        registry.addComponent<rtype::ecs::component::HitBox>(wall, config.width, config.height);
        registry.addComponent<rtype::ecs::component::Collidable>(wall, rtype::ecs::component::CollisionLayer::Obstacle);
        registry.addComponent<rtype::ecs::component::Drawable>(
            wall, "__RECTANGLE__", 0, 0, static_cast<int>(config.width), static_cast<int>(config.height));
    }

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float dt = clock.restart().asSeconds();

        auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(player);
        vel.vx = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            vel.vx = -400.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            vel.vx = 400.0f;

        physics_system->update(registry, dt);
        movement_system->update(registry, dt);

        renderer_impl->clear();
        render_system->update(registry, dt);
        renderer_impl->display();
    }

    return 0;
}
