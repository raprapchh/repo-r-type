#pragma once

namespace rtype::ecs::component {

struct HitBox {
    float width;
    float height;

    HitBox() : width(0.0f), height(0.0f) {
    }

    HitBox(float w, float h) : width(w), height(h) {
    }
};

} // namespace rtype::ecs::component
