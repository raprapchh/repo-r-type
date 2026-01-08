#pragma once

namespace rtype::ecs::component {

struct Score {
    int value = 0;
};

struct ScoreEvent {
    int points;

    ScoreEvent() : points(0) {
    }
    ScoreEvent(int p) : points(p) {
    }
};

} // namespace rtype::ecs::component
