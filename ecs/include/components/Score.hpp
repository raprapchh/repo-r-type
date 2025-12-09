#pragma once

namespace rtype::ecs::component {

struct Score {
    int value = 0;
};

struct ScoreEvent {
    int points;
};

} // namespace rtype::ecs::component
