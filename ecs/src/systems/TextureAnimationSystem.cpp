#include "systems/TextureAnimationSystem.hpp"
#include "components/TextureAnimation.hpp"
#include "components/Drawable.hpp"
#include <iostream>

namespace rtype::ecs {

void TextureAnimationSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Drawable, component::TextureAnimation>();

    for (auto entity : view) {
        auto& drawable = view.get<component::Drawable>(entity);
        auto& animation = view.get<component::TextureAnimation>(entity);

        if (animation.frameTextureNames.empty()) {
            continue;
        }

        animation.elapsedTime += static_cast<float>(dt);

        if (animation.elapsedTime >= animation.frameTime) {
            animation.elapsedTime = 0.0f;
            animation.currentFrameIndex++;

            if (animation.currentFrameIndex >= static_cast<int>(animation.frameTextureNames.size())) {
                if (animation.loop) {
                    animation.currentFrameIndex = 0;
                } else {
                    animation.currentFrameIndex = static_cast<int>(animation.frameTextureNames.size()) - 1;
                }
            }

            if (animation.currentFrameIndex >= 0 &&
                animation.currentFrameIndex < static_cast<int>(animation.frameTextureNames.size())) {
                drawable.texture_name = animation.frameTextureNames[animation.currentFrameIndex];

                drawable.current_sprite = 0;
                drawable.frame_count = 1;
            }
        }
    }
}

} // namespace rtype::ecs
