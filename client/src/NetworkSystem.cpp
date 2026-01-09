#include "../include/NetworkSystem.hpp"
#include "../../shared/net/MessageData.hpp"
#include "../../ecs/include/components/HitBox.hpp"
#include "../../ecs/include/components/CollisionLayer.hpp"
#include "../../shared/GameConstants.hpp"
#include <SFML/Graphics.hpp>
#include "../../ecs/include/components/Health.hpp"
#include "../../ecs/include/components/Projectile.hpp"
#include "../../ecs/include/components/Explosion.hpp"
#include "../../ecs/include/components/Weapon.hpp"
#include "../../ecs/include/components/Tag.hpp"
#include "../../ecs/include/components/NetworkInterpolation.hpp"
#include "../../ecs/include/components/AudioEvent.hpp"
#include <chrono>

namespace rtype::client {

NetworkSystem::NetworkSystem(uint32_t player_id) : player_id_(player_id) {
}

void NetworkSystem::push_packet(const rtype::net::Packet& packet) {
    std::lock_guard<std::mutex> lock(packet_queue_mutex_);
    packet_queue_.push(packet);
}

void NetworkSystem::set_player_id(uint32_t player_id) {
    player_id_ = player_id;
}

void NetworkSystem::update(GameEngine::Registry& registry, std::mutex& registry_mutex) {
    std::queue<rtype::net::Packet> packets_to_process;

    {
        std::lock_guard<std::mutex> lock(packet_queue_mutex_);
        packets_to_process = std::move(packet_queue_);
    }

    while (!packets_to_process.empty()) {
        rtype::net::Packet packet = packets_to_process.front();
        packets_to_process.pop();

        std::lock_guard<std::mutex> lock(registry_mutex);
        switch (static_cast<rtype::net::MessageType>(packet.header.message_type)) {
        case rtype::net::MessageType::EntitySpawn:
            handle_spawn(registry, packet);
            break;
        case rtype::net::MessageType::PlayerMove:
        case rtype::net::MessageType::EntityMove:
            handle_move(registry, packet);
            break;
        case rtype::net::MessageType::EntityDestroy:
            handle_destroy(registry, packet);
            break;
        default:
            break;
        }
    }
}

void NetworkSystem::handle_spawn(GameEngine::Registry& registry, const rtype::net::Packet& packet) {
    try {
        auto data = serializer_.deserialize_entity_spawn(packet);
        auto entity = registry.createEntity();

        registry.addComponent<rtype::ecs::component::NetworkId>(entity, data.entity_id);
        registry.addComponent<rtype::ecs::component::Position>(entity, data.position_x, data.position_y);
        registry.addComponent<rtype::ecs::component::Velocity>(entity, data.velocity_x, data.velocity_y);

        bool is_player = (data.entity_type == rtype::net::EntityType::PLAYER);
        bool is_local = (data.entity_id == player_id_);

        if (is_player) {
            uint32_t sprite_index = (data.entity_id - 1) % 4;
            registry.addComponent<rtype::ecs::component::Drawable>(
                entity, "player_ships", 0, 0, static_cast<uint32_t>(rtype::constants::PLAYER_WIDTH),
                static_cast<uint32_t>(rtype::constants::PLAYER_HEIGHT), rtype::constants::PLAYER_SCALE,
                rtype::constants::PLAYER_SCALE, 0, 0.1f, false, sprite_index, static_cast<uint32_t>(2));
            registry.addComponent<rtype::ecs::component::HitBox>(
                entity, rtype::constants::PLAYER_WIDTH * rtype::constants::PLAYER_SCALE,
                rtype::constants::PLAYER_HEIGHT * rtype::constants::PLAYER_SCALE);
            registry.addComponent<rtype::ecs::component::Collidable>(entity,
                                                                     rtype::ecs::component::CollisionLayer::Player);
            if (is_local) {
                registry.addComponent<rtype::ecs::component::Controllable>(entity, true);
            } else {
                registry.addComponent<rtype::ecs::component::NetworkInterpolation>(
                    entity, data.position_x, data.position_y, data.velocity_x, data.velocity_y);
            }
            registry.addComponent<rtype::ecs::component::Tag>(entity, "Player");
            registry.addComponent<rtype::ecs::component::HitBox>(entity, 165.0f, 110.0f);
        } else if (data.entity_type == rtype::net::EntityType::ENEMY) {
            std::string sprite_name = "enemy_basic";
            float scale = 4.0f;
            int frameW = 0;
            int frameH = 0;

            int frameCount = 1;
            if (data.sub_type == 1)
                sprite_name = "monster_0-top";
            else if (data.sub_type == 2)
                sprite_name = "monster_0-bot";
            else if (data.sub_type == 3)
                sprite_name = "monster_0-left";
            else if (data.sub_type == 4)
                sprite_name = "monster_0-right";
            else if (data.sub_type == 100) {
                sprite_name = "boss_1";
                frameW = 161;
                frameH = 219;
                scale = 2.0f;
                frameCount = 4;
            }

            registry.addComponent<rtype::ecs::component::Drawable>(
                entity, sprite_name, static_cast<uint32_t>(0), static_cast<uint32_t>(0), static_cast<uint32_t>(frameW),
                static_cast<uint32_t>(frameH), scale, scale, frameCount, 0.1f, true);

            float hp = 100.0f;
            float hitboxW = 100.0f;
            float hitboxH = 100.0f;

            if (data.sub_type == 100) {
                hp = 500.0f;
                hitboxW = 200.0f;
                hitboxH = 200.0f;
            }

            registry.addComponent<rtype::ecs::component::Health>(entity, hp, hp);
            registry.addComponent<rtype::ecs::component::HitBox>(entity, hitboxW, hitboxH);
            registry.addComponent<rtype::ecs::component::Collidable>(entity,
                                                                     rtype::ecs::component::CollisionLayer::Enemy);
            registry.addComponent<rtype::ecs::component::NetworkInterpolation>(entity, data.position_x, data.position_y,
                                                                               data.velocity_x, data.velocity_y);
        } else if (data.entity_type == rtype::net::EntityType::PROJECTILE) {
            std::string sprite_name = "shot";
            float width = 87.0f;
            float height = 99.0f;

            if (data.sub_type == 1) {
                sprite_name = "monster_0-ball";
                registry.addComponent<rtype::ecs::component::Drawable>(entity, sprite_name, static_cast<uint32_t>(0),
                                                                       static_cast<uint32_t>(0), 4.5f, 4.5f);
                width = 70.0f;
                height = 70.0f;
            } else if (data.sub_type >= 10 && data.sub_type <= 13) {
                int frameW = 0;
                int frameH = 0;
                if (data.sub_type == 10) {
                    sprite_name = "shot_death-charge1";
                    width = 60.0f;
                    height = 60.0f;
                } else if (data.sub_type == 11) {
                    sprite_name = "shot_death-charge2";
                    width = 80.0f;
                    height = 80.0f;
                } else if (data.sub_type == 12) {
                    sprite_name = "shot_death-charge3";
                    width = 100.0f;
                    height = 100.0f;
                } else if (data.sub_type == 13) {
                    sprite_name = "shot_death-charge4";
                    width = 120.0f;
                    height = 120.0f;
                }

                registry.addComponent<rtype::ecs::component::Drawable>(entity, sprite_name, 0, 0, frameW, frameH, 3.0f,
                                                                       3.0f, 2, 0.05f, false);
            } else if (data.sub_type == 20) {
                sprite_name = "boss_1_bayblade";
                width = 46.0f;
                height = 42.0f;
                registry.addComponent<rtype::ecs::component::Drawable>(entity, sprite_name, 0, 0, 23, 21, 2.0f, 2.0f, 4,
                                                                       0.1f, true);
            } else if (data.sub_type == 21) {
                sprite_name = "boss_1_attack";
                width = 42.0f;
                height = 40.0f;
                registry.addComponent<rtype::ecs::component::Drawable>(entity, sprite_name, 0, 0, 21, 20, 2.0f, 2.0f, 8,
                                                                       0.1f, true);
            } else if (data.sub_type == 30 || data.sub_type == 31) {
                // Force Pod Projectile
                if (data.sub_type == 31) {
                    sprite_name = "pod_projectile_red_0";
                    width = 36.0f;
                    height = 13.0f;
                } else {
                    sprite_name = "pod_projectile_0";
                    width = 34.0f;
                    height = 19.0f;
                }
                // Use 1 frame for now to match other logic, or let Renderer handle animation if it overrides
                registry.addComponent<rtype::ecs::component::Drawable>(entity, sprite_name, 0, 0, width, height, 2.5f,
                                                                       2.5f, 1, 0.1f, false);
            } else {
                registry.addComponent<rtype::ecs::component::Drawable>(entity, sprite_name, 0, 0, 29, 33, 3.0f, 3.0f, 4,
                                                                       0.05f, false);
            }
            registry.addComponent<rtype::ecs::component::Projectile>(entity, 10.0f, 5.0f);
            registry.addComponent<rtype::ecs::component::HitBox>(entity, width, height);

        } else if (data.entity_type == rtype::net::EntityType::OBSTACLE) {
            std::string sprite_name = "obstacle_1";
            if (data.sub_type == 1) {
                sprite_name = "floor_obstacle";
            }
            registry.addComponent<rtype::ecs::component::Drawable>(
                entity, sprite_name, static_cast<uint32_t>(0), static_cast<uint32_t>(0),
                rtype::constants::OBSTACLE_SCALE, rtype::constants::OBSTACLE_SCALE);
            float obs_w = rtype::constants::OBSTACLE_WIDTH;
            float obs_h = rtype::constants::OBSTACLE_HEIGHT;
            if (data.sub_type == 1) {
                obs_w = rtype::constants::FLOOR_OBSTACLE_WIDTH;
                obs_h = rtype::constants::FLOOR_OBSTACLE_HEIGHT;
            }

            registry.addComponent<rtype::ecs::component::HitBox>(entity, obs_w * rtype::constants::OBSTACLE_SCALE,
                                                                 obs_h * rtype::constants::OBSTACLE_SCALE);
            registry.addComponent<rtype::ecs::component::Collidable>(entity,
                                                                     rtype::ecs::component::CollisionLayer::Obstacle);

            if (data.sub_type == 1) {
                registry.addComponent<rtype::ecs::component::Tag>(entity, "Obstacle_Floor");
            } else {
                registry.addComponent<rtype::ecs::component::Tag>(entity, "Obstacle");
            }
        } else if (data.entity_type == rtype::net::EntityType::POWERUP) {
            std::string sprite_name = "force_pod";
            std::string tag_name = (data.sub_type == 1) ? "ForcePodItem" : "ForcePod";

            registry.addComponent<rtype::ecs::component::Drawable>(entity, sprite_name, static_cast<uint32_t>(0),
                                                                   static_cast<uint32_t>(0), static_cast<uint32_t>(0),
                                                                   static_cast<uint32_t>(0), 2.5f, 2.5f);
            registry.addComponent<rtype::ecs::component::HitBox>(entity, 64.0f, 64.0f);
            registry.addComponent<rtype::ecs::component::Collidable>(entity,
                                                                     rtype::ecs::component::CollisionLayer::PowerUp);
            registry.addComponent<rtype::ecs::component::Tag>(entity, tag_name);
        }

    } catch (const std::exception& e) {
    }
}

void NetworkSystem::handle_move(GameEngine::Registry& registry, const rtype::net::Packet& packet) {
    try {
        uint32_t entity_id = 0;
        float x = 0, y = 0;
        float vx = 0, vy = 0;

        if (static_cast<rtype::net::MessageType>(packet.header.message_type) == rtype::net::MessageType::PlayerMove) {
            auto data = serializer_.deserialize_player_move(packet);
            entity_id = data.player_id;
            x = data.position_x;
            y = data.position_y;
            vx = data.velocity_x;
            vy = data.velocity_y;
        } else {
            auto data = serializer_.deserialize_entity_move(packet);
            entity_id = data.entity_id;
            x = data.position_x;
            y = data.position_y;
            vx = data.velocity_x;
            vy = data.velocity_y;
        }

        auto view = registry.view<rtype::ecs::component::NetworkId>();
        bool found = false;

        for (auto entity : view) {
            GameEngine::entity_t entity_id_ecs = static_cast<GameEngine::entity_t>(entity);
            try {
                auto& net_id = registry.getComponent<rtype::ecs::component::NetworkId>(entity_id_ecs);
                if (net_id.id == entity_id) {
                    if (entity_id == player_id_) {
                        found = true;
                        break;
                    }

                    if (!registry.hasComponent<rtype::ecs::component::Position>(entity_id_ecs) ||
                        !registry.hasComponent<rtype::ecs::component::Velocity>(entity_id_ecs)) {
                        found = true;
                        break;
                    }

                    bool is_projectile = registry.hasComponent<rtype::ecs::component::Projectile>(entity_id_ecs);

                    if (is_projectile) {
                        auto& pos = registry.getComponent<rtype::ecs::component::Position>(entity_id_ecs);
                        auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(entity_id_ecs);
                        pos.x = x;
                        pos.y = y;
                        vel.vx = vx;
                        vel.vy = vy;
                    } else {
                        if (!registry.hasComponent<rtype::ecs::component::NetworkInterpolation>(entity_id_ecs)) {
                            auto& pos = registry.getComponent<rtype::ecs::component::Position>(entity_id_ecs);
                            registry.addComponent<rtype::ecs::component::NetworkInterpolation>(entity_id_ecs, pos.x,
                                                                                               pos.y, vx, vy);
                        }

                        auto& interp =
                            registry.getComponent<rtype::ecs::component::NetworkInterpolation>(entity_id_ecs);
                        interp.target_x = x;
                        interp.target_y = y;
                        interp.target_vx = vx;
                        interp.target_vy = vy;
                        interp.last_update_time = std::chrono::steady_clock::now();
                    }

                    found = true;
                    break;
                }
            } catch (const std::exception& e) {
                continue;
            }
        }

        if (!found) {
        }
    } catch (const std::exception& e) {
    }
}

void NetworkSystem::handle_destroy(GameEngine::Registry& registry, const rtype::net::Packet& packet) {
    try {
        auto data = serializer_.deserialize_entity_destroy(packet);

        auto view = registry.view<rtype::ecs::component::NetworkId>();

        for (auto entity : view) {
            GameEngine::entity_t entity_id_ecs = static_cast<GameEngine::entity_t>(entity);
            try {
                auto& net_id = registry.getComponent<rtype::ecs::component::NetworkId>(entity_id_ecs);
                if (net_id.id == data.entity_id) {
                    bool is_player = registry.hasComponent<rtype::ecs::component::Weapon>(entity_id_ecs);
                    bool is_enemy = !is_player && registry.hasComponent<rtype::ecs::component::Health>(entity_id_ecs);

                    if (is_player || is_enemy) {
                        float explosion_x = 0.0f;
                        float explosion_y = 0.0f;

                        if (registry.hasComponent<rtype::ecs::component::Position>(entity_id_ecs)) {
                            auto& pos = registry.getComponent<rtype::ecs::component::Position>(entity_id_ecs);
                            explosion_x = pos.x;
                            explosion_y = pos.y;
                        }

                        auto explosion_entity = registry.createEntity();
                        registry.addComponent<rtype::ecs::component::Position>(explosion_entity, explosion_x,
                                                                               explosion_y);
                        registry.addComponent<rtype::ecs::component::Explosion>(explosion_entity);
                        registry.addComponent<rtype::ecs::component::Drawable>(explosion_entity, "explosion", 5, 0, 37,
                                                                               44, 4.0f, 4.0f, 6, 0.1f, false);
                        auto& explosion_drawable =
                            registry.getComponent<rtype::ecs::component::Drawable>(explosion_entity);
                        constexpr int EXPLOSION_FRAME_WIDTH = 37;
                        constexpr int EXPLOSION_FRAME_HEIGHT = 44;
                        constexpr int EXPLOSION_FRAMES_PER_ROW = 6;
                        explosion_drawable.rect_x = 5;
                        explosion_drawable.rect_y = 0;
                        explosion_drawable.rect_width = EXPLOSION_FRAME_WIDTH;
                        explosion_drawable.rect_height = EXPLOSION_FRAME_HEIGHT;
                        explosion_drawable.frame_count = EXPLOSION_FRAMES_PER_ROW;
                        explosion_drawable.animation_speed = 0.1f;
                        explosion_drawable.loop = false;
                        explosion_drawable.current_sprite = 0;
                        explosion_drawable.animation_sequences["explosion"] = {5, 4, 3, 2, 1, 0};
                        explosion_drawable.current_state = "explosion";
                        explosion_drawable.animation_frame = 5;

                        // Add audio event for explosion
                        registry.addComponent<rtype::ecs::component::AudioEvent>(
                            explosion_entity, rtype::ecs::component::AudioEventType::EXPLOSION);
                    }
                    registry.destroyEntity(entity_id_ecs);
                    return;
                }
            } catch (const std::exception& e) {
                continue;
            }
        }
    } catch (const std::exception& e) {
    }
}

} // namespace rtype::client
