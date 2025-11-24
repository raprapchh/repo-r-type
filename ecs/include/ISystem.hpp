#pragma once

#include "Registry.hpp"

namespace rtype::ecs {

class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void run(Registry& registry) = 0;
};

} // namespace rtype::ecs
