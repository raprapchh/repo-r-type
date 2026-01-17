#include <SFML/Graphics.hpp>

#include <memory>
#include <iostream>
#include <vector>

#include "Registry.hpp"
#include "components/Position.hpp"
#include "components/Velocity.hpp"
#include "components/Drawable.hpp"
#include "components/Controllable.hpp"
#include "components/HitBox.hpp"
#include "components/CollisionLayer.hpp"
#include "components/Tag.hpp"
#include "components/Gravity.hpp"
#include "components/Jump.hpp"
#include "SFMLRenderer.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/PlatformerPhysicsSystem.hpp"
#include "systems/WeaponSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "components/Weapon.hpp"
#include "components/Projectile.hpp"
#include "systems/MapGeneratorSystem.hpp"

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

    sf::Texture player_up_texture;
    if (!player_up_texture.loadFromFile("platformer/assets/player_up.png") &&
        !player_up_texture.loadFromFile("assets/player_up.png") &&
        !player_up_texture.loadFromFile("../platformer/assets/player_up.png") &&
        !player_up_texture.loadFromFile("../assets/player_up.png")) {
        std::cerr << "Failed to load player_up texture!" << std::endl;
    } else {
        textures["player_up"] = player_up_texture;
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

    sf::Texture projectile_texture;
    if (!projectile_texture.loadFromFile("platformer/assets/projectile.png") &&
        !projectile_texture.loadFromFile("assets/projectile.png") &&
        !projectile_texture.loadFromFile("../platformer/assets/projectile.png") &&
        !projectile_texture.loadFromFile("../assets/projectile.png")) {
        std::cerr << "Failed to load projectile texture!" << std::endl;
    } else {
        textures["projectile"] = projectile_texture;
    }

    sf::Texture hole_texture;
    if (!hole_texture.loadFromFile("platformer/assets/hole@2x.png") &&
        !hole_texture.loadFromFile("assets/hole@2x.png") &&
        !hole_texture.loadFromFile("../platformer/assets/hole@2x.png") &&
        !hole_texture.loadFromFile("../assets/hole@2x.png")) {
        std::cerr << "Failed to load hole texture!" << std::endl;
    } else {
        textures["hole"] = hole_texture;
    }

    auto renderer_impl = std::make_shared<rtype::rendering::SFMLRenderer>(window, textures);
    auto render_system = std::make_shared<rtype::ecs::RenderSystem>(renderer_impl, nullptr);

    auto movement_system = std::make_shared<rtype::ecs::MovementSystem>();
    auto physics_system = std::make_shared<rtype::ecs::PlatformerPhysicsSystem>();
    auto weapon_system = std::make_shared<rtype::ecs::WeaponSystem>();
    auto projectile_system = std::make_shared<rtype::ecs::ProjectileSystem>();
    auto map_generator_system = std::make_shared<rtype::ecs::MapGeneratorSystem>();

    auto player = registry.createEntity();
    registry.addComponent<rtype::ecs::component::Position>(player, 200.0f, 400.0f);
    registry.addComponent<rtype::ecs::component::Velocity>(player, 0.0f, 0.0f);
    registry.addComponent<rtype::ecs::component::HitBox>(player, 124.0f, 120.0f);
    registry.addComponent<rtype::ecs::component::Drawable>(player, "player_left", 0, 0, 0, 0);
    registry.addComponent<rtype::ecs::component::Controllable>(player);
    registry.addComponent<rtype::ecs::component::Gravity>(player);
    registry.addComponent<rtype::ecs::component::Jump>(player);
    registry.addComponent<rtype::ecs::component::Jump>(player);
    registry.addComponent<rtype::ecs::component::Tag>(player, "PlatformerPlayer");

    auto& weapon = registry.addComponent<rtype::ecs::component::Weapon>(player);
    weapon.projectileTag = "projectile";
    weapon.projectileWidth = 16.0f;
    weapon.projectileHeight = 16.0f;
    weapon.ignoreScroll = true;
    weapon.fireRate = 0.2f;
    weapon.projectileSpeed = 1200.0f;
    weapon.projectileLifetime = 2.0f;
    weapon.directionX = 0.0f;
    weapon.directionY = -1.0f;
    weapon.damage = 10.0f;
    weapon.spawnOffsetX = 62.0f;
    weapon.spawnOffsetY = 0.0f;

    auto start_plat = registry.createEntity();
    registry.addComponent<rtype::ecs::component::Position>(start_plat, 150.0f, 550.0f);
    registry.addComponent<rtype::ecs::component::HitBox>(start_plat, 500.0f, 20.0f);
    registry.addComponent<rtype::ecs::component::Collidable>(start_plat,
                                                             rtype::ecs::component::CollisionLayer::Obstacle);
    registry.addComponent<rtype::ecs::component::Drawable>(start_plat, "green_platform", 0, 0, 500, 20);
    registry.addComponent<rtype::ecs::component::Tag>(start_plat, "Platform");

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

    sf::Texture title_texture;
    if (!title_texture.loadFromFile("platformer/assets/doodle-jump@2x.png") &&
        !title_texture.loadFromFile("assets/doodle-jump@2x.png") &&
        !title_texture.loadFromFile("../platformer/assets/doodle-jump@2x.png") &&
        !title_texture.loadFromFile("../assets/doodle-jump@2x.png")) {
        std::cerr << "Failed to load title texture!" << std::endl;
    }

    sf::Sprite title_sprite(title_texture);
    sf::FloatRect titleRect = title_sprite.getLocalBounds();
    title_sprite.setOrigin(titleRect.width / 2.0f, titleRect.height / 2.0f);
    title_sprite.setPosition(400, 200);

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

            static std::string last_horizontal_facing = "player_right";
            if (registry.getComponent<rtype::ecs::component::Drawable>(player).texture_name == "player_left") {
                last_horizontal_facing = "player_left";
            }
            if (registry.getComponent<rtype::ecs::component::Drawable>(player).texture_name == "player_right") {
                last_horizontal_facing = "player_right";
            }

            bool moving_horizontal = false;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                vel.vx = -400.0f;
                registry.getComponent<rtype::ecs::component::Drawable>(player).texture_name = "player_left";
                last_horizontal_facing = "player_left";
                moving_horizontal = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                vel.vx = 400.0f;
                registry.getComponent<rtype::ecs::component::Drawable>(player).texture_name = "player_right";
                last_horizontal_facing = "player_right";
                moving_horizontal = true;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                registry.getComponent<rtype::ecs::component::Drawable>(player).texture_name = "player_up";
            } else if (!moving_horizontal) {
                registry.getComponent<rtype::ecs::component::Drawable>(player).texture_name = last_horizontal_facing;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                auto& weapon = registry.getComponent<rtype::ecs::component::Weapon>(player);
                weapon.isShooting = true;

                float dirX = 0.0f;
                float dirY = 0.0f;

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                    dirY = -1.0f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                    dirX = -1.0f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                    dirX = 1.0f;

                if (dirX == 0.0f && dirY == 0.0f) {
                    auto& drawable = registry.getComponent<rtype::ecs::component::Drawable>(player);
                    if (drawable.texture_name == "player_left") {
                        dirX = -1.0f;
                    } else {
                        dirX = 1.0f;
                    }
                }

                weapon.directionX = dirX;
                weapon.directionY = dirY;

                float spawnX = 62.0f - 8.0f;
                float spawnY = 60.0f - 8.0f;

                if (dirX > 0)
                    spawnX = 124.0f;
                if (dirX < 0)
                    spawnX = -16.0f;
                if (dirY > 0)
                    spawnY = 120.0f;
                if (dirY < 0)
                    spawnY = -16.0f;

                weapon.spawnOffsetX = spawnX;
                weapon.spawnOffsetY = spawnY;
            } else {
                registry.getComponent<rtype::ecs::component::Weapon>(player).isShooting = false;
            }

            physics_system->update(registry, dt);
            movement_system->update(registry, dt);
            weapon_system->update(registry, dt);
            projectile_system->update(registry, dt);
            map_generator_system->update(registry, view.getCenter().y);

            auto& pos = registry.getComponent<rtype::ecs::component::Position>(player);
            auto& hitbox = registry.getComponent<rtype::ecs::component::HitBox>(player);

            if (pos.x > 800) {
                pos.x = -hitbox.width;
            } else if (pos.x < -hitbox.width) {
                pos.x = 800;
            }

            auto hole_view =
                registry
                    .view<rtype::ecs::component::Tag, rtype::ecs::component::Position, rtype::ecs::component::HitBox>();
            for (auto hole_entity : hole_view) {
                auto& tag = registry.getComponent<rtype::ecs::component::Tag>(hole_entity);
                if (tag.name == "Hole") {
                    auto& holePos = registry.getComponent<rtype::ecs::component::Position>(hole_entity);
                    auto& holeHitbox = registry.getComponent<rtype::ecs::component::HitBox>(hole_entity);

                    float shrinkX = 60.0f;
                    float shrinkY = 60.0f;

                    if (pos.x < holePos.x + holeHitbox.width - shrinkX && pos.x + hitbox.width > holePos.x + shrinkX &&
                        pos.y < holePos.y + holeHitbox.height - shrinkY &&
                        pos.y + hitbox.height > holePos.y + shrinkY) {
                        game_over = true;
                    }
                }
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
            window.draw(title_sprite);
            window.draw(play_sprite);
        } else if (!game_over) {
            render_system->update(registry, 0);

            auto projectiles = registry.view<rtype::ecs::component::Projectile, rtype::ecs::component::Position>();
            for (auto entity : projectiles) {
                auto& pos = registry.getComponent<rtype::ecs::component::Position>(entity);
                sf::CircleShape circle(5.0f);
                circle.setFillColor(sf::Color::Black);
                circle.setPosition(pos.x, pos.y);
                window.draw(circle);
            }
        } else {
            lose_text.setPosition(view.getCenter());
            window.draw(lose_text);
        }
        renderer_impl->display();
    }

    return 0;
}
