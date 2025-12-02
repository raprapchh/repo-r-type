#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"
#include "../components/CollisionLayer.hpp"

namespace rtype::ecs {

class CollisionSystem : public ISystem {
  public:
    ~CollisionSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;

  private:
    static bool CheckAABBCollision(float x1, float y1, float w1, float h1,
                                   float x2, float y2, float w2, float h2);

    static bool ShouldCollide(component::CollisionLayer layer1, component::CollisionLayer layer2);

    void HandleCollision(GameEngine::Registry& registry,
                        GameEngine::entity_t entity1,
                        GameEngine::entity_t entity2,
                        component::CollisionLayer layer1,
                        component::CollisionLayer layer2);
};

} // namespace rtype::ecs
