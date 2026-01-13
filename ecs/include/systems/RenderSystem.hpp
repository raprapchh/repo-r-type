#pragma once

#include "interfaces/ecs/ISystem.hpp"
#include "interfaces/rendering/IRenderer.hpp"
#include "../Registry.hpp"
#include <memory>

namespace rtype::client {
class AccessibilityManager;
}

namespace rtype::ecs {

class RenderSystem : public ISystem {
  public:
    explicit RenderSystem(std::shared_ptr<rtype::rendering::IRenderer> renderer = nullptr,
                          rtype::client::AccessibilityManager* accessibility_mgr = nullptr);

    ~RenderSystem() override = default;

    void update(GameEngine::Registry& registry, double dt) override;

    void set_renderer(std::shared_ptr<rtype::rendering::IRenderer> renderer);

    std::shared_ptr<rtype::rendering::IRenderer> get_renderer() const;

  private:
    std::shared_ptr<rtype::rendering::IRenderer> renderer_;
    rtype::client::AccessibilityManager* accessibility_manager_;
};

} // namespace rtype::ecs
