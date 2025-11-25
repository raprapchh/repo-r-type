#pragma once

namespace GameEngine {
class Registry;
}

namespace rtype::ecs {

class ISystem {
  public:
    virtual ~ISystem() = default;

    /**
     * @brief Executes system logic.
     * @param registry The entity registry.
     */
    virtual void update(GameEngine::Registry& registry) = 0;
};

} // namespace rtype::ecs
