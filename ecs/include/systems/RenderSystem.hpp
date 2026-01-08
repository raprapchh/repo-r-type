#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../../shared/interfaces/rendering/IRenderer.hpp"
#include "../Registry.hpp"
#include <memory>

namespace rtype::client {
class AccessibilityManager;
}

namespace rtype::ecs {

/**
 * @class RenderSystem
 * @brief Headless-capable render system that iterates over entities with Position and Drawable components
 *
 * This system is decoupled from SFML and uses an abstract IRenderer interface.
 * It handles sprite animation, texture mapping, and rendering without dependency on a specific graphics library.
 * The system can operate in headless mode when no renderer is provided.
 */
class RenderSystem : public ISystem {
  public:
    /**
     * @brief Construct a RenderSystem with an optional renderer
     * @param renderer Pointer to an IRenderer implementation (can be nullptr for headless mode)
     * @param accessibility_mgr Optional accessibility manager for entity coloring
     */
    explicit RenderSystem(std::shared_ptr<rtype::rendering::IRenderer> renderer = nullptr,
                          rtype::client::AccessibilityManager* accessibility_mgr = nullptr);

    ~RenderSystem() override = default;

    /**
     * @brief Update the render system
     * Iterates over entities with Position and Drawable components and renders them.
     * Handles animation frames and cleanup of finished animations.
     *
     * @param registry The entity registry
     * @param dt Delta time since last update
     */
    void update(GameEngine::Registry& registry, double dt) override;

    /**
     * @brief Set the renderer for this system
     * @param renderer Pointer to an IRenderer implementation
     */
    void set_renderer(std::shared_ptr<rtype::rendering::IRenderer> renderer);

    /**
     * @brief Get the current renderer
     * @return Shared pointer to the current renderer
     */
    std::shared_ptr<rtype::rendering::IRenderer> get_renderer() const;

  private:
    std::shared_ptr<rtype::rendering::IRenderer> renderer_;
    rtype::client::AccessibilityManager* accessibility_manager_;
};

} // namespace rtype::ecs
