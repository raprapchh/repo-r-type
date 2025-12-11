#include "Client.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include "../../ecs/include/components/NetworkId.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Velocity.hpp"
#include "../../ecs/include/components/Drawable.hpp"
#include "../../ecs/include/components/Controllable.hpp"
#include "../../ecs/include/components/HitBox.hpp"
#include "../../ecs/include/components/Lives.hpp"
#include "../../ecs/include/components/Health.hpp"
#include "../../ecs/include/components/Tag.hpp"
#include <iostream>
#include <chrono>

namespace rtype::client {

Client::Client(const std::string& host, uint16_t port, Renderer& renderer)
    : host_(host), port_(port), connected_(false), player_id_(0), renderer_(renderer), network_system_(0) {
    io_context_ = std::make_unique<asio::io_context>();
    udp_client_ = std::make_unique<UdpClient>(*io_context_, host_, port_);
    udp_client_->set_message_handler(
        [this](const asio::error_code& error, std::size_t bytes_transferred, const std::vector<uint8_t>& data) {
            handle_udp_receive(error, bytes_transferred, data);
        });
}

void Client::set_game_start_callback(GameStartCallback callback) {
    game_start_callback_ = callback;
}

void Client::set_player_join_callback(PlayerJoinCallback callback) {
    player_join_callback_ = callback;
}

Client::~Client() {
    disconnect();
}

void Client::connect() {
    asio::ip::udp::resolver resolver(*io_context_);
    rtype::net::MessageSerializer serializer;
    rtype::net::PlayerJoinData join_data;
    rtype::net::Packet join_packet = serializer.serialize_player_join(join_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(join_packet);

    udp_client_->send(packet_data);
    std::cout << "Connection request sent to " << host_ << ":" << port_ << std::endl;

    udp_client_->start_receive();
    run();
}

void Client::disconnect() {
    bool was_connected = connected_.load();
    connected_ = false;

    if (io_context_) {
        io_context_->stop();
    }

    if (network_thread_ && network_thread_->joinable()) {
        network_thread_->join();
    }

    if (udp_client_) {
        udp_client_->stop();
    }

    if (was_connected) {
        std::cout << "Disconnected from server." << std::endl;
    }
}

void Client::run() {
    if (!network_thread_ || !network_thread_->joinable()) {
        network_thread_ = std::make_unique<std::thread>([this]() { io_context_->run(); });
    }
}

void Client::handle_udp_receive(const asio::error_code& error, std::size_t bytes_transferred,
                                const std::vector<uint8_t>& data) {
    (void)bytes_transferred;
    if (!error) {
        handle_server_message(data);
    } else {
        std::cerr << "Receive error: " << error.message() << std::endl;
        if (error != asio::error::operation_aborted) {
            disconnect();
        }
    }
}

void Client::handle_server_message(const std::vector<uint8_t>& data) {
    rtype::net::ProtocolAdapter adapter;
    if (!adapter.validate(data)) {
        std::cerr << "Error: Invalid packet received (protocol validation failed)." << std::endl;
        return;
    }
    rtype::net::Packet packet = adapter.deserialize(data);
    rtype::net::MessageSerializer serializer;

    if (packet.header.payload_size != packet.body.size()) {
        std::cerr << "Error: Malformed packet received. Announced body size (" << packet.header.payload_size
                  << ") does not match actual body size (" << packet.body.size() << ")." << std::endl;
        return;
    }

    switch (static_cast<rtype::net::MessageType>(packet.header.message_type)) {
    case rtype::net::MessageType::PlayerJoin: {
        try {
            auto join_data = serializer.deserialize_player_join(packet);
            if (!connected_.load()) {
                player_id_ = join_data.player_id;
                network_system_.set_player_id(player_id_);
                std::lock_guard<std::mutex> lock(registry_mutex_);
                auto entity = registry_.createEntity();
                registry_.addComponent<rtype::ecs::component::NetworkId>(entity, player_id_);
                registry_.addComponent<rtype::ecs::component::Position>(entity, 100.0f, 100.0f);
                registry_.addComponent<rtype::ecs::component::Velocity>(entity, 0.0f, 0.0f);
                uint32_t sprite_index = (player_id_ - 1) % 4;
                registry_.addComponent<rtype::ecs::component::Drawable>(entity, "player_ships", 0, 0, 33, 0, 5.0f, 5.0f,
                                                                        0, 0.1f, false, sprite_index,
                                                                        static_cast<uint32_t>(2));
                registry_.addComponent<rtype::ecs::component::Controllable>(entity, true);
                auto& drawable = registry_.getComponent<rtype::ecs::component::Drawable>(entity);
                registry_.addComponent<rtype::ecs::component::HitBox>(entity, 165.0f, 110.0f);
                registry_.addComponent<rtype::ecs::component::Tag>(entity, "Player");
                registry_.addComponent<rtype::ecs::component::Lives>(entity, 3);
                registry_.addComponent<rtype::ecs::component::Health>(entity, 100, 100);
                drawable.animation_sequences["idle"] = {2};
                drawable.animation_sequences["up"] = {2, 3, 4};
                drawable.animation_sequences["down"] = {2, 1, 0};
                drawable.current_state = "idle";
                drawable.last_state = "idle";
                drawable.animation_timer = 0.0f;
                drawable.animation_speed = 0.1f;
                drawable.current_sprite = 2;
                drawable.animation_frame = 0;

                connected_ = true;
                std::cout << "Successfully connected to server. My Player ID is " << player_id_ << std::endl;
            } else {
                std::cout << "Player " << join_data.player_id << " has joined the game." << std::endl;

                if (join_data.player_id != player_id_) {
                    std::lock_guard<std::mutex> lock(registry_mutex_);
                    auto entity = registry_.createEntity();
                    registry_.addComponent<rtype::ecs::component::NetworkId>(entity, join_data.player_id);
                    registry_.addComponent<rtype::ecs::component::Position>(entity, 100.0f, 100.0f);
                    registry_.addComponent<rtype::ecs::component::Velocity>(entity, 0.0f, 0.0f);
                    uint32_t sprite_index = (join_data.player_id - 1) % 4;
                    registry_.addComponent<rtype::ecs::component::Drawable>(entity, "player_ships", 0, 0, 33, 0, 5.0f,
                                                                            5.0f, 0, 0.1f, false, sprite_index,
                                                                            static_cast<uint32_t>(2));
                    auto& drawable = registry_.getComponent<rtype::ecs::component::Drawable>(entity);
                    registry_.addComponent<rtype::ecs::component::HitBox>(entity, 96.0f, 96.0f);
                    registry_.addComponent<rtype::ecs::component::Lives>(entity, 3);
                    registry_.addComponent<rtype::ecs::component::Health>(entity, 100, 100);
                    drawable.animation_sequences["idle"] = {2};
                    drawable.animation_sequences["up"] = {2, 3, 4};
                    drawable.animation_sequences["down"] = {2, 1, 0};
                    drawable.current_state = "idle";
                    drawable.last_state = "idle";
                    drawable.animation_timer = 0.0f;
                    drawable.animation_speed = 0.1f;
                    drawable.current_sprite = 2;
                    drawable.animation_frame = 0;
                }

                if (player_join_callback_) {
                    player_join_callback_(join_data.player_id);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing PlayerJoin packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::GameStart: {
        try {
            auto game_start_data = serializer.deserialize_game_start(packet);
            std::cout << "Game starting! Session: " << game_start_data.session_id
                      << ", Players: " << static_cast<int>(game_start_data.player_count) << std::endl;
            if (game_start_callback_) {
                game_start_callback_();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing GameStart packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::GameState: {
        try {
            if (packet.body.size() < 15) {
                std::cerr << "Error: GameState packet too small" << std::endl;
                break;
            }
            auto game_state_data = serializer.deserialize_game_state(packet);
            renderer_.update_game_state(game_state_data);

            {
                std::lock_guard<std::mutex> lock(registry_mutex_);
                auto view = registry_.view<rtype::ecs::component::NetworkId>();
                for (auto entity : view) {
                    auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(
                        static_cast<GameEngine::entity_t>(entity));
                    if (net_id.id == player_id_) {
                        if (registry_.hasComponent<rtype::ecs::component::Lives>(
                                static_cast<GameEngine::entity_t>(entity))) {
                            auto& lives = registry_.getComponent<rtype::ecs::component::Lives>(
                                static_cast<GameEngine::entity_t>(entity));
                            lives.remaining = game_state_data.lives;
                        }
                        break;
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing GameState packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::PlayerMove: {
        network_system_.push_packet(packet);
        try {
            auto move_data = serializer.deserialize_player_move(packet);

            if (move_data.player_id == player_id_) {
                return;
            }

            std::lock_guard<std::mutex> lock(registry_mutex_);
            bool found = false;
            GameEngine::entity_t found_entity_id = 0;

            {
                auto view = registry_.view<rtype::ecs::component::NetworkId>();
                for (auto entity : view) {
                    GameEngine::entity_t entity_id = static_cast<GameEngine::entity_t>(entity);
                    try {
                        auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(entity_id);
                        if (net_id.id == move_data.player_id) {
                            found_entity_id = entity_id;
                            found = true;
                            break;
                        }
                    } catch (const std::exception& e) {
                        continue;
                    }
                }
            }

            if (found) {
                try {
                    if (registry_.hasComponent<rtype::ecs::component::Position>(found_entity_id)) {
                        auto& pos = registry_.getComponent<rtype::ecs::component::Position>(found_entity_id);
                        pos.x = move_data.position_x;
                        pos.y = move_data.position_y;
                    }
                    if (registry_.hasComponent<rtype::ecs::component::Velocity>(found_entity_id)) {
                        auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(found_entity_id);
                        vel.vx = move_data.velocity_x;
                        vel.vy = move_data.velocity_y;
                    }
                    if (registry_.hasComponent<rtype::ecs::component::Drawable>(found_entity_id)) {
                        auto& drawable = registry_.getComponent<rtype::ecs::component::Drawable>(found_entity_id);
                        auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(found_entity_id);

                        const float threshold = 0.5f;

                        if (drawable.animation_sequences.empty()) {
                            drawable.animation_sequences["idle"] = {2};
                            drawable.animation_sequences["up"] = {2, 3, 4};
                            drawable.animation_sequences["down"] = {2, 1, 0};
                        }

                        std::string new_state = drawable.current_state;

                        if (vel.vy < -threshold) {
                            new_state = "up";
                        } else if (vel.vy > threshold) {
                            new_state = "down";
                        } else {
                            new_state = "idle";
                        }

                        if (new_state != drawable.last_state) {
                            drawable.current_state = new_state;
                            drawable.animation_timer = 0.0f;
                            drawable.animation_frame = 0;

                            drawable.last_state = new_state;
                        }
                    }
                } catch (const std::exception& e) {
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing PlayerMove packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::EntitySpawn: {
        network_system_.push_packet(packet);
        break;
    }

    case rtype::net::MessageType::EntityMove: {
        network_system_.push_packet(packet);
        break;
    }

    case rtype::net::MessageType::EntityDestroy: {
        network_system_.push_packet(packet);
        break;
    }

    case rtype::net::MessageType::Pong: {
        try {
            auto pong_data = serializer.deserialize_ping_pong(packet);
            (void)pong_data;
            std::cout << "Received Pong." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing Pong packet: " << e.what() << std::endl;
        }
        break;
    }
    default:
        std::cerr << "Warning: Unknown message type received: " << static_cast<int>(packet.header.message_type)
                  << std::endl;
        break;
    }
}

void Client::send_move(float vx, float vy) {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::PlayerMoveData move_data;
    move_data.player_id = player_id_;
    move_data.position_x = 0.0f;
    move_data.position_y = 0.0f;
    move_data.velocity_x = vx;
    move_data.velocity_y = vy;
    rtype::net::Packet move_packet = serializer.serialize_player_move(move_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(move_packet);
    udp_client_->send(packet_data);
}

void Client::send_shoot(int32_t x, int32_t y) {
    (void)x;
    (void)y;
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::PlayerShootData shoot_data;
    shoot_data.player_id = player_id_;

    float pos_x = 0.0f;
    float pos_y = 0.0f;
    {
        std::lock_guard<std::mutex> lock(registry_mutex_);
        auto view = registry_.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position>();
        for (auto entity : view) {
            auto& net_id =
                registry_.getComponent<rtype::ecs::component::NetworkId>(static_cast<GameEngine::entity_t>(entity));
            if (net_id.id == player_id_) {
                auto& pos =
                    registry_.getComponent<rtype::ecs::component::Position>(static_cast<GameEngine::entity_t>(entity));
                pos_x = pos.x;
                pos_y = pos.y;
                break;
            }
        }
    }

    shoot_data.weapon_type = 0;
    shoot_data.position_x = pos_x;
    shoot_data.position_y = pos_y;
    shoot_data.direction_x = 1.0f;
    shoot_data.direction_y = 0.0f;
    rtype::net::Packet shoot_packet = serializer.serialize_player_shoot(shoot_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(shoot_packet);
    udp_client_->send(packet_data);
}

void Client::send_game_start_request() {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::GameStartData start_data;
    start_data.session_id = 1;
    start_data.level_id = 1;
    start_data.player_count = 0;
    start_data.difficulty = 1;
    start_data.timestamp = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    rtype::net::Packet start_packet = serializer.serialize_game_start(start_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(start_packet);
    udp_client_->send(packet_data);
}

void Client::send_map_resize(float width, float height) {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::MapResizeData resize_data(width, height);
    rtype::net::Packet resize_packet = serializer.serialize_map_resize(resize_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(resize_packet);
    udp_client_->send(packet_data);
}

void Client::update(double dt) {
    audio_system_.update(registry_, dt);
    network_system_.update(registry_, registry_mutex_);

    std::lock_guard<std::mutex> lock(registry_mutex_);
    auto view = registry_.view<rtype::ecs::component::Drawable, rtype::ecs::component::Velocity>();
    for (auto entity : view) {
        auto& drawable =
            registry_.getComponent<rtype::ecs::component::Drawable>(static_cast<GameEngine::entity_t>(entity));
        auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(static_cast<GameEngine::entity_t>(entity));

        if (drawable.animation_sequences.empty())
            continue;

        if (vel.vy < 0)
            drawable.current_state = "up";
        else if (vel.vy > 0)
            drawable.current_state = "down";
        else
            drawable.current_state = "idle";

        if (drawable.current_state != drawable.last_state) {
            drawable.animation_frame = 0;
            const auto& seq = drawable.animation_sequences[drawable.current_state];
            if (!seq.empty())
                drawable.current_sprite = seq[0];
            drawable.animation_timer = 0.0f;
            drawable.last_state = drawable.current_state;
            continue;
        }

        const auto& seq = drawable.animation_sequences[drawable.current_state];
        if (seq.empty())
            continue;

        drawable.animation_timer += static_cast<float>(dt);
        if (drawable.animation_timer >= drawable.animation_speed) {
            drawable.animation_timer = 0.0f;
            if (drawable.loop) {
                drawable.animation_frame = (drawable.animation_frame + 1) % seq.size();
            } else {
                if (drawable.animation_frame + 1 < seq.size())
                    drawable.animation_frame++;
            }
            drawable.current_sprite = seq[drawable.animation_frame];
            if (drawable.current_sprite >= 5) {
                std::cerr << "Invalid sprite index detected: " << drawable.current_sprite << std::endl;
                drawable.current_sprite = 2;
            }
        }
    }
}

} // namespace rtype::client
