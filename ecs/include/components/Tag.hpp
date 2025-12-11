#pragma once

#include <string>

namespace rtype::ecs::component {

struct Tag {
    std::string name;

    Tag(const std::string& n = "") : name(n) {
    }
};

} // namespace rtype::ecs::component
