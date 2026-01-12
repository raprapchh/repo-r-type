#pragma once

#include <vector>
#include <string>

namespace rtype::ecs::component {

struct TextureAnimation {
    std::vector<std::string> frameTextureNames;
    float frameTime;
    float elapsedTime;
    int currentFrameIndex;
    bool loop;

    TextureAnimation(const std::vector<std::string>& frames, float time, bool loopAnim = true)
        : frameTextureNames(frames), frameTime(time), elapsedTime(0.0f), currentFrameIndex(0), loop(loopAnim) {
    }

    TextureAnimation() : frameTime(0.1f), elapsedTime(0.0f), currentFrameIndex(0), loop(true) {
    }
};

} // namespace rtype::ecs::component
