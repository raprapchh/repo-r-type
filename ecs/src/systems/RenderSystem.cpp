#include "../../include/systems/RenderSystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Drawable.hpp"
#include <SFML/Graphics.hpp>

namespace rtype::ecs {

RenderSystem::RenderSystem(sf::RenderWindow& window, std::unordered_map<std::string, sf::Texture>& textures)
    : window_(window), textures_(textures) {
}

void RenderSystem::update(GameEngine::Registry& registry, double /* dt */) {
    auto view = registry.view<component::Position, component::Drawable>();

    for (auto entity : view) {
        auto& pos = registry.getComponent<component::Position>(static_cast<size_t>(entity));
        auto& drawable = registry.getComponent<component::Drawable>(static_cast<size_t>(entity));

        if (textures_.find(drawable.texture_name) == textures_.end()) {
            continue;
        }

        const sf::Texture& texture = textures_.at(drawable.texture_name);
        sf::Vector2u texture_size = texture.getSize();

        uint32_t sprite_width = texture_size.x / 5;
        uint32_t sprite_height = texture_size.y / 5;

        uint32_t row = drawable.sprite_index;
        uint32_t col = drawable.animation_frame % 5;

        sf::IntRect texture_rect(
            col * sprite_width,
            row * sprite_height,
            sprite_width,
            sprite_height
        );

        sf::Sprite sprite(texture, texture_rect);
        sprite.setPosition(pos.x, pos.y);
        sprite.setScale(drawable.scale_x, drawable.scale_y);

        window_.draw(sprite);
    }
}

} // namespace rtype::ecs

