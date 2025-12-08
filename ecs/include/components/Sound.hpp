#pragma once

#include <string>

namespace rtype::ecs {

struct Sound {
    std::string sound_id;
    bool loop = false;
    bool should_play = false;
};

} // namespace rtype::ecs
