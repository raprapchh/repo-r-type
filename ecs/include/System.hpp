#pragma once

#include "ISystem.hpp"

namespace rtype::ecs {

class System : public ISystem {
public:
    virtual ~System() = default;
    virtual void run(Registry& registry) = 0;
};

} // namespace rtype::ecs
