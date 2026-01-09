#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <cstdint>
#include "../../shared/net/MessageData.hpp"
#include "AccessibilityManager.hpp"

namespace rtype::client {

struct Entity {
    uint32_t id;
    uint16_t type;
    float x;
    float y;
    float velocity_x;
    float velocity_y;
    int animation_frame = 2;
    float animation_timer = 0.0f;
    int player_state = 0;
    uint16_t sub_type = 0;
    float hit_flash_timer = 0.0f; // Timer for hit flash effect
};

class Renderer {
  public:
    Renderer(uint32_t width = 1280, uint32_t height = 720);
    ~Renderer();

    bool is_open() const;
    bool poll_event(sf::Event& event);
    void clear();
    void display();

    void handle_input();
    void update_entity(const Entity& entity);
    void remove_entity(uint32_t entity_id);
    void update_game_state(const rtype::net::GameStateData& state);
    void spawn_entity(const Entity& entity);
    void update_animations(float delta_time);
    void close_window();

    void draw_entities();
    void draw_ui();
    void draw_background();
    void draw_game_over(bool all_players_dead);
    bool is_game_over_back_to_menu_clicked(const sf::Vector2f& mouse_pos) const;
    void render_frame();

    sf::Vector2f get_player_position(uint32_t player_id) const;

    bool is_moving_up() const {
        return keys_[sf::Keyboard::Up];
    }
    bool is_moving_down() const {
        return keys_[sf::Keyboard::Down];
    }
    bool is_moving_left() const {
        return keys_[sf::Keyboard::Left];
    }
    bool is_moving_right() const {
        return keys_[sf::Keyboard::Right];
    }
    bool is_shooting() const {
        return keys_[sf::Keyboard::Space];
    }

    sf::Vector2f get_shoot_direction() const;

    void draw_text(const sf::Text& text);
    void draw_rectangle(const sf::RectangleShape& rectangle);
    sf::Vector2u get_window_size() const;
    sf::Vector2f get_mouse_position() const;
    void handle_resize(uint32_t width, uint32_t height);
    void set_charge_percentage(float percentage) {
        charge_percentage_ = percentage;
    }

    std::unordered_map<std::string, sf::Texture>& get_textures() {
        return textures_;
    }

    sf::RenderWindow* get_window() {
        return window_.get();
    }

    AccessibilityManager& get_accessibility_manager() {
        return accessibility_manager_;
    }

  private:
    void load_sprites();
    void load_fonts();
    void load_texture(const std::string& path, const std::string& name);
    sf::Sprite create_sprite(const Entity& entity);

    std::unique_ptr<sf::RenderWindow> window_;
    sf::View view_;
    std::unordered_map<std::string, sf::Texture> textures_;
    std::unordered_map<uint32_t, Entity> entities_;
    sf::Font font_;
    sf::Text score_text_;
    sf::Text lives_text_;
    sf::Text wave_text_;

    bool keys_[sf::Keyboard::KeyCount];
    float background_x_;
    float background_x_stars_;
    float background_x_stars2_;

    rtype::net::GameStateData game_state_;
    mutable sf::RectangleShape back_to_menu_button_;
    mutable sf::Text back_to_menu_text_;
    float charge_percentage_ = 0.0f;
    sf::Sprite charge_particle_sprite_;
    int charge_particle_frame_ = 0;
    float charge_particle_timer_ = 0.0f;
    AccessibilityManager accessibility_manager_;

    bool stage_cleared_ = false;
    uint8_t cleared_stage_number_ = 1;
    float stage_cleared_timer_ = 0.0f;

  public:
    void draw_charge_effect(const sf::Vector2f& position, float delta_time);
    void show_stage_cleared(uint8_t stage_number);
    void draw_stage_cleared();
    bool is_stage_cleared() const {
        return stage_cleared_;
    }
};

} // namespace rtype::client
