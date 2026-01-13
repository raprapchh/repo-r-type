#include <SFML/Graphics.hpp>

#include <memory>
#include <iostream>
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
    sf::RenderWindow window(sf::VideoMode(800, 600), "Doodle Jump");
    window.setFramerateLimit(60);

    sf::View view(sf::FloatRect(0, 0, 800, 600));

    std::unordered_map<std::string, sf::Texture> textures;
    sf::Texture player_left_texture;
    if (!player_left_texture.loadFromFile("platformer/assets/player_left.png") &&
        !player_left_texture.loadFromFile("assets/player_left.png") &&
        !player_left_texture.loadFromFile("../platformer/assets/player_left.png") &&
        !player_left_texture.loadFromFile("../assets/player_left.png")) {
        std::cerr << "Failed to load player_left texture!" << std::endl;
    } else {
        textures["player_left"] = player_left_texture;
    }

    sf::Texture player_right_texture;
    if (!player_right_texture.loadFromFile("platformer/assets/player_right.png") &&
        !player_right_texture.loadFromFile("assets/player_right.png") &&
        !player_right_texture.loadFromFile("../platformer/assets/player_right.png") &&
        !player_right_texture.loadFromFile("../assets/player_right.png")) {
        std::cerr << "Failed to load player_right texture!" << std::endl;
    } else {
        textures["player_right"] = player_right_texture;
    }

    sf::Texture bck_texture;
    if (!bck_texture.loadFromFile("platformer/assets/bck.png") && !bck_texture.loadFromFile("assets/bck.png") &&
        !bck_texture.loadFromFile("../platformer/assets/bck.png") && !bck_texture.loadFromFile("../assets/bck.png")) {
    } else {
        bck_texture.setRepeated(true);
        textures["bck"] = bck_texture;
    }

    sf::Texture green_platform_texture;
    if (!green_platform_texture.loadFromFile("platformer/assets/green_platform.png") &&
        !green_platform_texture.loadFromFile("assets/green_platform.png") &&
        !green_platform_texture.loadFromFile("../platformer/assets/green_platform.png") &&
        !green_platform_texture.loadFromFile("../assets/green_platform.png")) {
        std::cerr << "Failed to load green_platform texture!" << std::endl;
    } else {
        textures["green_platform"] = green_platform_texture;
    }

    auto renderer_impl = std::make_shared<rtype::rendering::SFMLRenderer>(window, textures);
    auto render_system = std::make_shared<rtype::ecs::RenderSystem>(renderer_impl, nullptr);

    auto movement_system = std::make_shared<rtype::ecs::MovementSystem>();
    auto physics_system = std::make_shared<rtype::ecs::PlatformerPhysicsSystem>();

    auto player = registry.createEntity();
    registry.addComponent<rtype::ecs::component::Position>(player, 200.0f, 400.0f);
    registry.addComponent<rtype::ecs::component::Velocity>(player, 0.0f, 0.0f);
    registry.addComponent<rtype::ecs::component::HitBox>(player, 124.0f, 120.0f);
    registry.addComponent<rtype::ecs::component::Drawable>(player, "player_left", 0, 0, 0, 0);
    registry.addComponent<rtype::ecs::component::Controllable>(player);
    registry.addComponent<rtype::ecs::component::Gravity>(player);
    registry.addComponent<rtype::ecs::component::Jump>(player);
    registry.addComponent<rtype::ecs::component::Tag>(player, "Player");

    struct PlatformConfig {
        float x, y, width, height;
    };

    std::vector<PlatformConfig> level_layout = {
        {0.0f, 550.0f, 800.0f, 50.0f},     {200.0f, 450.0f, 100.0f, 20.0f},   {500.0f, 350.0f, 100.0f, 20.0f},
        {100.0f, 250.0f, 100.0f, 20.0f},   {600.0f, 150.0f, 100.0f, 20.0f},   {300.0f, 50.0f, 100.0f, 20.0f},
        {100.0f, -50.0f, 100.0f, 20.0f},   {600.0f, -150.0f, 100.0f, 20.0f},  {350.0f, -250.0f, 100.0f, 20.0f},
        {50.0f, -350.0f, 100.0f, 20.0f},   {550.0f, -450.0f, 100.0f, 20.0f},  {250.0f, -550.0f, 100.0f, 20.0f},
        {700.0f, -650.0f, 100.0f, 20.0f},  {150.0f, -750.0f, 100.0f, 20.0f},  {450.0f, -850.0f, 100.0f, 20.0f},
        {50.0f, -950.0f, 100.0f, 20.0f},   {600.0f, -1050.0f, 100.0f, 20.0f}, {300.0f, -1150.0f, 100.0f, 20.0f},
        {100.0f, -1250.0f, 100.0f, 20.0f}, {500.0f, -1350.0f, 100.0f, 20.0f}, {200.0f, -1450.0f, 100.0f, 20.0f},
        {650.0f, -1550.0f, 100.0f, 20.0f}, {400.0f, -1650.0f, 100.0f, 20.0f}};

    for (const auto& config : level_layout) {
        auto wall = registry.createEntity();
        registry.addComponent<rtype::ecs::component::Position>(wall, config.x, config.y);
        registry.addComponent<rtype::ecs::component::HitBox>(wall, config.width, config.height);
        registry.addComponent<rtype::ecs::component::Collidable>(wall, rtype::ecs::component::CollisionLayer::Obstacle);
        registry.addComponent<rtype::ecs::component::Drawable>(
            wall, "green_platform", 0, 0, static_cast<int>(config.width), static_cast<int>(config.height));
    }

    sf::Font font;
    if (!font.loadFromFile("client/fonts/Ethnocentric-Regular.otf") &&
        !font.loadFromFile("../client/fonts/Ethnocentric-Regular.otf") &&
        !font.loadFromFile("../../client/fonts/Ethnocentric-Regular.otf") &&
        !font.loadFromFile("fonts/Ethnocentric-Regular.otf")) {
        std::cerr << "Failed to load font from any common path!" << std::endl;
        return 84;
    }
    sf::Text lose_text("You Lost", font, 50);
    lose_text.setFillColor(sf::Color::Red);

    sf::FloatRect textRect = lose_text.getLocalBounds();
    lose_text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);

    sf::Text title_text("Doodle Jump", font, 60);
    title_text.setFillColor(sf::Color::Blue);
    sf::FloatRect titleRect = title_text.getLocalBounds();
    title_text.setOrigin(titleRect.left + titleRect.width / 2.0f, titleRect.top + titleRect.height / 2.0f);
    title_text.setPosition(400, 200);

    sf::Texture play_texture;
    if (!play_texture.loadFromFile("platformer/assets/play@2x.png") &&
        !play_texture.loadFromFile("assets/play@2x.png") &&
        !play_texture.loadFromFile("../platformer/assets/play@2x.png") &&
        !play_texture.loadFromFile("../assets/play@2x.png")) {
        std::cerr << "Failed to load play texture!" << std::endl;
    }
    sf::Texture play_on_texture;
    if (!play_on_texture.loadFromFile("platformer/assets/play-on@2x.png") &&
        !play_on_texture.loadFromFile("assets/play-on@2x.png") &&
        !play_on_texture.loadFromFile("../platformer/assets/play-on@2x.png") &&
        !play_on_texture.loadFromFile("../assets/play-on@2x.png")) {
        std::cerr << "Failed to load play-on texture!" << std::endl;
    }

    sf::Sprite play_sprite(play_texture);
    sf::FloatRect playRect = play_sprite.getLocalBounds();
    play_sprite.setOrigin(playRect.width / 2.0f, playRect.height / 2.0f);
    play_sprite.setPosition(400, 400);

    bool in_menu = true;
    bool game_over = false;

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (in_menu && event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
                    if (play_sprite.getGlobalBounds().contains(worldPos)) {
                        in_menu = false;
                        clock.restart();
                    }
                }
            }
        }

        if (in_menu) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
            if (play_sprite.getGlobalBounds().contains(worldPos)) {
                play_sprite.setTexture(play_on_texture);
            } else {
                play_sprite.setTexture(play_texture);
            }
        }

        if (!in_menu && !game_over) {
            float dt = clock.restart().asSeconds();

            auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(player);
            vel.vx = 0;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                vel.vx = -400.0f;
                registry.getComponent<rtype::ecs::component::Drawable>(player).texture_name = "player_left";
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                vel.vx = 400.0f;
                registry.getComponent<rtype::ecs::component::Drawable>(player).texture_name = "player_right";
            }

            physics_system->update(registry, dt);
            movement_system->update(registry, dt);

            auto& pos = registry.getComponent<rtype::ecs::component::Position>(player);
            auto& hitbox = registry.getComponent<rtype::ecs::component::HitBox>(player);

            if (pos.x > 800) {
                pos.x = -hitbox.width;
            } else if (pos.x < -hitbox.width) {
                pos.x = 800;
            }

            if (pos.y < view.getCenter().y) {
                view.setCenter(400, pos.y);
                window.setView(view);
            }

            if (pos.y > view.getCenter().y + 300) {
                game_over = true;
            }
        }

        renderer_impl->clear();
        renderer_impl->draw_parallax_background("bck", view.getCenter().y);
        if (in_menu) {
            window.setView(window.getDefaultView());
            window.draw(title_text);
            window.draw(play_sprite);
        } else if (!game_over) {
            render_system->update(registry, 0);
        } else {
            lose_text.setPosition(view.getCenter());
            window.draw(lose_text);
        }
        renderer_impl->display();
    }

    return 0;
}
