#include "Client.hpp"
#include "net/ProtocolAdapter.hpp"
#include "net/MessageSerializer.hpp"
#include "GameConstants.hpp"
#include "components/NetworkId.hpp"
#include "components/Position.hpp"
#include "components/Velocity.hpp"
#include "components/Drawable.hpp"
#include "components/Controllable.hpp"
#include "components/HitBox.hpp"
#include "components/CollisionLayer.hpp"
#include "components/Lives.hpp"
#include "components/Health.hpp"
#include "components/Score.hpp"
#include "components/Tag.hpp"
#include "components/NetworkInterpolation.hpp"
#include "components/PingStats.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/TextureAnimationSystem.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstring>

namespace rtype::client {

Client::Client(const std::string& host, uint16_t port, Renderer& renderer)
    : connected_(false), player_id_(0), network_system_(0), renderer_(renderer), host_(host), port_(port),
      last_ping_time_(std::chrono::steady_clock::now()) {
    io_context_ = std::make_unique<asio::io_context>();
    udp_client_ = std::make_unique<UdpClient>(*io_context_, host_, port_);
    udp_client_->set_message_handler(
        [this](const asio::error_code& error, std::size_t bytes_transferred, const std::vector<uint8_t>& data) {
            handle_udp_receive(error, bytes_transferred, data);
        });

    scoreboard_manager_.load();

    audio_system_.initializeAudioAssets();

    // Initialize systems
    system_manager_.addSystem<rtype::ecs::MovementSystem>();
    system_manager_.addSystem<rtype::ecs::TextureAnimationSystem>();
}

void Client::set_game_start_callback(std::function<void()> callback) {
    game_start_callback_ = callback;
}

void Client::set_player_join_callback(std::function<void(uint32_t, const std::string&)> callback) {
    player_join_callback_ = callback;
    for (const auto& player : pending_players_) {
        if (player_join_callback_) {
            player_join_callback_(player.first, player.second);
        }
    }
    pending_players_.clear();
}

Client::~Client() {
    audio_system_.cleanup();
    disconnect();
}

void Client::connect() {
    asio::ip::udp::resolver resolver(*io_context_);

    std::cout << "UDP connection initialized to " << host_ << ":" << port_ << std::endl;

    udp_client_->start_receive();
    run();
}

void Client::disconnect() {
    bool was_connected = connected_.load();

    if (was_connected && udp_client_) {
        try {
            rtype::net::MessageSerializer serializer;
            rtype::net::PlayerLeaveData leave_data(player_id_);
            rtype::net::Packet leave_packet = serializer.serialize_player_leave(leave_data);
            std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(leave_packet);

            for (int i = 0; i < 3; ++i) {
                udp_client_->send(packet_data);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        } catch (const std::exception& e) {
            std::cerr << "Error sending PlayerLeave: " << e.what() << std::endl;
        }
    }

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

void Client::reconnect() {
    // Stop existing connections
    if (io_context_) {
        io_context_->stop();
    }

    if (network_thread_ && network_thread_->joinable()) {
        network_thread_->join();
    }

    if (udp_client_) {
        udp_client_->stop();
    }

    // Clear all state
    connected_ = false;
    session_id_ = 0;
    player_id_ = 0;
    network_system_.set_player_id(0);
    network_system_.clear_packet_queue();

    {
        std::lock_guard<std::mutex> lock(registry_mutex_);
        registry_.clear();
    }

    // Recreate io_context and UDP client
    io_context_ = std::make_unique<asio::io_context>();
    udp_client_ = std::make_unique<UdpClient>(*io_context_, host_, port_);
    udp_client_->set_message_handler(
        [this](const asio::error_code& error, std::size_t bytes_transferred, const std::vector<uint8_t>& data) {
            handle_udp_receive(error, bytes_transferred, data);
        });

    std::cout << "Client reconnected - ready for new session" << std::endl;
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
        if (connected_.load()) {
            std::cerr << "Error: Invalid packet received (protocol validation failed)." << std::endl;
        }
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
                session_id_ = join_data.session_id;
                player_id_ = join_data.player_id;
                network_system_.set_player_id(player_id_);
                std::lock_guard<std::mutex> lock(registry_mutex_);
                auto entity = registry_.createEntity();
                registry_.addComponent<rtype::ecs::component::NetworkId>(entity, player_id_);
                registry_.addComponent<rtype::ecs::component::Position>(entity, 100.0f, 100.0f);
                registry_.addComponent<rtype::ecs::component::Velocity>(entity, 0.0f, 0.0f);
                uint32_t sprite_index = (player_id_ - 1) % 4;
                registry_.addComponent<rtype::ecs::component::Drawable>(
                    entity, "player_ships", 0, 0, static_cast<uint32_t>(rtype::constants::PLAYER_WIDTH),
                    static_cast<uint32_t>(rtype::constants::PLAYER_HEIGHT), rtype::constants::PLAYER_SCALE,
                    rtype::constants::PLAYER_SCALE, 0, 0.1f, false, sprite_index, static_cast<uint32_t>(2));
                registry_.addComponent<rtype::ecs::component::Controllable>(entity, true);
                auto& drawable = registry_.getComponent<rtype::ecs::component::Drawable>(entity);
                registry_.addComponent<rtype::ecs::component::HitBox>(
                    entity, rtype::constants::PLAYER_WIDTH * rtype::constants::PLAYER_SCALE,
                    rtype::constants::PLAYER_HEIGHT * rtype::constants::PLAYER_SCALE);
                registry_.addComponent<rtype::ecs::component::Collidable>(
                    entity, rtype::ecs::component::CollisionLayer::Player);
                registry_.addComponent<rtype::ecs::component::Tag>(entity, "Player");
                registry_.addComponent<rtype::ecs::component::Lives>(entity, 3);
                registry_.addComponent<rtype::ecs::component::Health>(entity, 100, 100);
                registry_.addComponent<rtype::ecs::component::Score>(entity, 0);
                registry_.addComponent<rtype::ecs::component::NetworkInterpolation>(entity, 100.0f, 100.0f, 0.0f, 0.0f);
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
                std::cout << "Successfully connected to server. Room " << session_id_ << " - My Player ID is "
                          << player_id_ << std::endl;
            } else {
                std::cout << "Player " << join_data.player_id << " has joined the game." << std::endl;

                std::lock_guard<std::mutex> lock(registry_mutex_);
                bool player_exists = false;
                auto view = registry_.view<rtype::ecs::component::NetworkId>();
                for (auto entity : view) {
                    auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(
                        static_cast<GameEngine::entity_t>(entity));
                    if (net_id.id == join_data.player_id) {
                        player_exists = true;
                        break;
                    }
                }

                if (!player_exists) {
                    auto entity = registry_.createEntity();
                    registry_.addComponent<rtype::ecs::component::NetworkId>(entity, join_data.player_id);
                    registry_.addComponent<rtype::ecs::component::Position>(entity, 100.0f, 100.0f);
                    registry_.addComponent<rtype::ecs::component::Velocity>(entity, 0.0f, 0.0f);
                    uint32_t sprite_index = (join_data.player_id - 1) % 4;
                    registry_.addComponent<rtype::ecs::component::Drawable>(
                        entity, "player_ships", 0, 0, static_cast<uint32_t>(rtype::constants::PLAYER_WIDTH),
                        static_cast<uint32_t>(rtype::constants::PLAYER_HEIGHT), rtype::constants::PLAYER_SCALE,
                        rtype::constants::PLAYER_SCALE, 0, 0.1f, false, sprite_index, static_cast<uint32_t>(2));
                    auto& drawable = registry_.getComponent<rtype::ecs::component::Drawable>(entity);
                    registry_.addComponent<rtype::ecs::component::HitBox>(
                        entity, rtype::constants::PLAYER_WIDTH * rtype::constants::PLAYER_SCALE,
                        rtype::constants::PLAYER_HEIGHT * rtype::constants::PLAYER_SCALE);
                    registry_.addComponent<rtype::ecs::component::Collidable>(
                        entity, rtype::ecs::component::CollisionLayer::Player);
                    registry_.addComponent<rtype::ecs::component::Tag>(entity, "Player");
                    registry_.addComponent<rtype::ecs::component::Lives>(entity, 3);
                    registry_.addComponent<rtype::ecs::component::Health>(entity, 100, 100);

                    if (join_data.player_id == player_id_) {
                        registry_.addComponent<rtype::ecs::component::Controllable>(entity, true);
                    } else {
                        registry_.addComponent<rtype::ecs::component::NetworkInterpolation>(entity, 100.0f, 100.0f,
                                                                                            0.0f, 0.0f);
                    }

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

                std::string player_name = "Player " + std::to_string(join_data.player_id);
                if (join_data.player_name[0] != '\0') {
                    player_name = std::string(join_data.player_name);
                }

                if (player_join_callback_) {
                    player_join_callback_(join_data.player_id, player_name);
                } else {
                    pending_players_.emplace_back(join_data.player_id, player_name);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing PlayerJoin packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::PlayerName: {
        try {
            auto name_data = serializer.deserialize_player_name(packet);
            std::string name(name_data.player_name);
            if (player_name_callback_) {
                player_name_callback_(name_data.player_id, name);
            } else {
                for (auto& p : pending_players_) {
                    if (p.first == name_data.player_id) {
                        p.second = name;
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing PlayerName packet: " << e.what() << std::endl;
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
                        if (registry_.hasComponent<rtype::ecs::component::Score>(
                                static_cast<GameEngine::entity_t>(entity))) {
                            auto& score = registry_.getComponent<rtype::ecs::component::Score>(
                                static_cast<GameEngine::entity_t>(entity));
                            score.value = game_state_data.score;
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
                            bool is_player = false;
                            if (registry_.hasComponent<rtype::ecs::component::Tag>(entity_id)) {
                                auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(entity_id);
                                if (tag.name == "Player") {
                                    is_player = true;
                                }
                            }
                            if (is_player) {
                                found_entity_id = entity_id;
                                found = true;
                                break;
                            }
                        }
                    } catch (const std::exception& e) {
                        continue;
                    }
                }
            }

            if (!found) {
                auto entity = registry_.createEntity();
                registry_.addComponent<rtype::ecs::component::NetworkId>(entity, move_data.player_id);
                registry_.addComponent<rtype::ecs::component::Position>(entity, move_data.position_x,
                                                                        move_data.position_y);
                registry_.addComponent<rtype::ecs::component::Velocity>(entity, move_data.velocity_x,
                                                                        move_data.velocity_y);
                uint32_t sprite_index = (move_data.player_id - 1) % 4;
                registry_.addComponent<rtype::ecs::component::Drawable>(
                    entity, "player_ships", 0, 0, static_cast<uint32_t>(rtype::constants::PLAYER_WIDTH),
                    static_cast<uint32_t>(rtype::constants::PLAYER_HEIGHT), rtype::constants::PLAYER_SCALE,
                    rtype::constants::PLAYER_SCALE, 0, 0.1f, false, sprite_index, static_cast<uint32_t>(2));
                auto& drawable = registry_.getComponent<rtype::ecs::component::Drawable>(entity);
                registry_.addComponent<rtype::ecs::component::HitBox>(
                    entity, rtype::constants::PLAYER_WIDTH * rtype::constants::PLAYER_SCALE,
                    rtype::constants::PLAYER_HEIGHT * rtype::constants::PLAYER_SCALE);
                registry_.addComponent<rtype::ecs::component::Collidable>(
                    entity, rtype::ecs::component::CollisionLayer::Player);
                registry_.addComponent<rtype::ecs::component::Tag>(entity, "Player");
                registry_.addComponent<rtype::ecs::component::Lives>(entity, 3);
                registry_.addComponent<rtype::ecs::component::Health>(entity, 100, 100);

                if (move_data.player_id == player_id_) {
                    registry_.addComponent<rtype::ecs::component::Controllable>(entity, true);
                }
                registry_.addComponent<rtype::ecs::component::NetworkInterpolation>(
                    entity, move_data.position_x, move_data.position_y, move_data.velocity_x, move_data.velocity_y);
                drawable.animation_sequences["idle"] = {2};
                drawable.animation_sequences["up"] = {2, 3, 4};
                drawable.animation_sequences["down"] = {2, 1, 0};
                drawable.current_state = "idle";
                drawable.last_state = "idle";
                drawable.animation_timer = 0.0f;
                drawable.animation_speed = 0.1f;
                drawable.current_sprite = 2;
                drawable.animation_frame = 0;
            } else {
                try {
                    if (registry_.hasComponent<rtype::ecs::component::Position>(found_entity_id) &&
                        registry_.hasComponent<rtype::ecs::component::Velocity>(found_entity_id)) {
                        auto& pos = registry_.getComponent<rtype::ecs::component::Position>(found_entity_id);
                        auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(found_entity_id);

                        bool is_local_player = (move_data.player_id == player_id_);

                        if (!registry_.hasComponent<rtype::ecs::component::NetworkInterpolation>(found_entity_id)) {
                            registry_.addComponent<rtype::ecs::component::NetworkInterpolation>(
                                found_entity_id, move_data.position_x, move_data.position_y, move_data.velocity_x,
                                move_data.velocity_y);
                            pos.x = move_data.position_x;
                            pos.y = move_data.position_y;
                            vel.vx = move_data.velocity_x;
                            vel.vy = move_data.velocity_y;
                        } else {
                            auto& interp =
                                registry_.getComponent<rtype::ecs::component::NetworkInterpolation>(found_entity_id);
                            interp.target_x = move_data.position_x;
                            interp.target_y = move_data.position_y;
                            interp.target_vx = move_data.velocity_x;
                            interp.target_vy = move_data.velocity_y;
                            interp.last_update_time = std::chrono::steady_clock::now();

                            if (is_local_player) {
                                pos.x = move_data.position_x;
                                pos.y = move_data.position_y;
                                vel.vx = move_data.velocity_x;
                                vel.vy = move_data.velocity_y;
                            }
                        }
                    }
                    if (registry_.hasComponent<rtype::ecs::component::Drawable>(found_entity_id)) {
                        auto& drawable = registry_.getComponent<rtype::ecs::component::Drawable>(found_entity_id);

                        const float threshold = 0.5f;

                        if (drawable.animation_sequences.empty()) {
                            drawable.animation_sequences["idle"] = {2};
                            drawable.animation_sequences["up"] = {2, 3, 4};
                            drawable.animation_sequences["down"] = {2, 1, 0};
                        }

                        std::string new_state = drawable.current_state;

                        if (move_data.velocity_y < -threshold) {
                            new_state = "up";
                        } else if (move_data.velocity_y > threshold) {
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

    case rtype::net::MessageType::PlayerLeave: {
        try {
            auto leave_data = serializer.deserialize_player_leave(packet);
            std::cout << "Player " << leave_data.player_id << " has left the game." << std::endl;

            GameEngine::entity_t entity_to_destroy = static_cast<GameEngine::entity_t>(-1);
            {
                std::lock_guard<std::mutex> lock(registry_mutex_);
                auto view = registry_.view<rtype::ecs::component::NetworkId>();
                for (auto entity : view) {
                    auto& net_id = registry_.getComponent<rtype::ecs::component::NetworkId>(
                        static_cast<GameEngine::entity_t>(entity));
                    if (net_id.id == leave_data.player_id) {
                        entity_to_destroy = static_cast<GameEngine::entity_t>(entity);
                        break;
                    }
                }
            }

            if (entity_to_destroy != static_cast<GameEngine::entity_t>(-1)) {
                std::lock_guard<std::mutex> lock(registry_mutex_);
                registry_.destroyEntity(entity_to_destroy);
                std::cout << "Removed entity for player " << leave_data.player_id << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error handling PlayerLeave: " << e.what() << std::endl;
        }
        break;
    }

    case rtype::net::MessageType::Pong: {
        try {
            auto pong_data = serializer.deserialize_ping_pong(packet);

            // Calculate RTT
            auto now = std::chrono::steady_clock::now();
            uint64_t sent_time_us = pong_data.timestamp;
            auto sent_time_pt =
                std::chrono::time_point<std::chrono::steady_clock>(std::chrono::microseconds(sent_time_us));
            auto rtt_us = std::chrono::duration_cast<std::chrono::microseconds>(now - sent_time_pt).count();
            float rtt_ms = static_cast<float>(rtt_us) / 1000.0f;

            if (rtt_ms < 0)
                rtt_ms = 0.0f;

            // Update PingStats component
            {
                std::lock_guard<std::mutex> lock(registry_mutex_);
                auto view = registry_.view<rtype::ecs::component::PingStats>();
                for (auto entity : view) {
                    auto& stats = registry_.getComponent<rtype::ecs::component::PingStats>(entity);
                    stats.lastPingMs = rtt_ms;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing Pong packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::ChatMessage: {
        try {
            auto chat_data = serializer.deserialize_chat_message(packet);
            std::string sender_name(chat_data.player_name);
            std::string message(chat_data.message);
            if (chat_message_callback_) {
                chat_message_callback_(chat_data.player_id, sender_name, message);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing ChatMessage packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::RoomInfo: {
        try {
            auto room_data = serializer.deserialize_room_info(packet);
            std::string room_name(room_data.room_name);
            if (!pending_create_room_name_.empty() && room_name == pending_create_room_name_) {
                // Save the new session id and send PlayerJoin
                session_id_ = room_data.session_id;
                rtype::net::PlayerJoinData join_data(session_id_, 0, player_name_);
                rtype::net::Packet join_packet = serializer.serialize_player_join(join_data);
                std::vector<uint8_t> join_packet_data = rtype::net::ProtocolAdapter().serialize(join_packet);
                udp_client_->send(join_packet_data);
                pending_create_room_name_.clear();
            }
            if (room_list_callback_) {
                room_list_callback_(room_data.session_id, room_data.player_count, room_data.max_players,
                                    room_data.status, room_name);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing RoomInfo packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::StageCleared: {
        try {
            if (packet.body.size() >= sizeof(rtype::net::StageClearedData)) {
                rtype::net::StageClearedData stage_data;
                std::memcpy(&stage_data, packet.body.data(), sizeof(rtype::net::StageClearedData));
                std::cout << "[VICTORY] Stage " << static_cast<int>(stage_data.stage_number) << " Cleared!"
                          << std::endl;
                renderer_.show_stage_cleared(stage_data.stage_number);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing StageCleared packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::LobbyUpdate: {
        try {
            auto lobby_data = serializer.deserialize_lobby_update(packet);
            if (lobby_update_callback_) {
                lobby_update_callback_(lobby_data.playerCount, lobby_data.yourPlayerId);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing LobbyUpdate packet: " << e.what() << std::endl;
        }
        break;
    }
    case rtype::net::MessageType::RestartVoteStatus: {
        try {
            auto status_data = serializer.deserialize_restart_vote_status(packet);
            renderer_.update_restart_vote_status(status_data);
        } catch (const std::exception& e) {
            std::cerr << "Error deserializing RestartVoteStatus packet: " << e.what() << std::endl;
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

void Client::send_shoot(int32_t x, int32_t y, int chargeLevel) {
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

    shoot_data.weapon_type = static_cast<uint16_t>(chargeLevel);
    shoot_data.position_x = pos_x;
    shoot_data.position_y = pos_y;
    shoot_data.direction_x = 1.0f;
    shoot_data.direction_y = 0.0f;
    rtype::net::Packet shoot_packet = serializer.serialize_player_shoot(shoot_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(shoot_packet);
    udp_client_->send(packet_data);
}

void Client::send_ping(uint64_t timestamp) {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::PingPongData ping_data(timestamp);
    rtype::net::Packet ping_packet = serializer.serialize_ping(ping_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(ping_packet);
    udp_client_->send(packet_data);
}

void Client::send_restart_vote(bool play_again) {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::RestartVoteData vote_data(player_id_, play_again ? 1 : 0);
    rtype::net::Packet vote_packet = serializer.serialize_restart_vote(vote_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(vote_packet);
    udp_client_->send(packet_data);
}

void Client::send_game_start_request() {
    if (!connected_.load())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::GameStartData start_data;
    start_data.session_id = session_id_;
    start_data.level_id = 1;
    start_data.player_count = 0;
    start_data.difficulty = 1;
    start_data.timestamp = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    rtype::net::Packet start_packet = serializer.serialize_game_start(start_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(start_packet);
    udp_client_->send(packet_data);
}

void Client::send_player_name_update(const std::string& name) {
    if (!connected_.load())
        return;

    // Update local player name
    player_name_ = name;

    if (message_serializer_) {
        // Send player name update
        rtype::net::PlayerNameData name_data;
        name_data.player_id = player_id_;
        std::strncpy(name_data.player_name, name.c_str(), sizeof(name_data.player_name) - 1);
        name_data.player_name[sizeof(name_data.player_name) - 1] = '\0';

        auto packet = message_serializer_->serialize_player_name(name_data);
        if (protocol_adapter_) {
            auto bytes = protocol_adapter_->serialize(packet);
            udp_client_->send(bytes);
        }
    }
}

std::string Client::get_player_name() const {
    return player_name_;
}

uint32_t Client::get_player_id() const {
    return player_id_;
}

void Client::set_player_name_callback(std::function<void(uint32_t, const std::string&)> callback) {
    player_name_callback_ = callback;
}

void Client::set_chat_message_callback(std::function<void(uint32_t, const std::string&, const std::string&)> callback) {
    chat_message_callback_ = callback;
}

void Client::set_room_list_callback(
    std::function<void(uint32_t, uint8_t, uint8_t, uint8_t, const std::string&)> callback) {
    room_list_callback_ = callback;
}

void Client::set_lobby_update_callback(std::function<void(int8_t, int8_t)> callback) {
    lobby_update_callback_ = callback;
}

void Client::request_room_list() {
    rtype::net::MessageSerializer serializer;
    rtype::net::ListRoomsData list_data;
    rtype::net::Packet list_packet = serializer.serialize_list_rooms(list_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(list_packet);
    udp_client_->send(packet_data);
}

void Client::create_room(const std::string& room_name, uint8_t max_players, rtype::config::GameMode mode,
                         rtype::config::Difficulty difficulty, bool friendly_fire, uint8_t lives) {
    rtype::net::MessageSerializer serializer;
    rtype::net::CreateRoomData create_data(room_name, max_players, static_cast<uint8_t>(mode),
                                           static_cast<uint8_t>(difficulty),
                                           friendly_fire ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0), lives);
    rtype::net::Packet create_packet = serializer.serialize_create_room(create_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(create_packet);
    udp_client_->send(packet_data);
    // Remember room name to auto-join when RoomInfo arrives with its new session_id
    pending_create_room_name_ = room_name;
    std::cout << "Create room request sent: " << room_name << std::endl;
}

void Client::join_room(uint32_t session_id) {
    session_id_ = session_id;

    // Send PlayerJoin with the correct session_id
    rtype::net::MessageSerializer serializer;
    rtype::net::PlayerJoinData join_data(session_id_, 0, player_name_);
    rtype::net::Packet join_packet = serializer.serialize_player_join(join_data);
    std::vector<uint8_t> join_packet_data = rtype::net::ProtocolAdapter().serialize(join_packet);
    udp_client_->send(join_packet_data);
    std::cout << "Join room " << session_id << " request sent" << std::endl;
}

void Client::send_chat_message(const std::string& message) {
    if (!connected_.load() || message.empty())
        return;
    rtype::net::MessageSerializer serializer;
    rtype::net::ChatMessageData chat_data(player_id_, player_name_, message);
    rtype::net::Packet chat_packet = serializer.serialize_chat_message(chat_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(chat_packet);
    udp_client_->send(packet_data);
}

void Client::leave_room() {
    if (!connected_.load())
        return;

    // Send PlayerLeave message to server
    rtype::net::MessageSerializer serializer;
    rtype::net::PlayerLeaveData leave_data(player_id_);
    rtype::net::Packet leave_packet = serializer.serialize_player_leave(leave_data);
    std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(leave_packet);
    udp_client_->send(packet_data);

    // Reset local session markers but keep UDP socket alive
    session_id_ = 0;
    player_id_ = 0;
    connected_ = false; // allow next PlayerJoin to re-init state
    network_system_.set_player_id(0);

    // Drop local entities to avoid leftovers on next join
    {
        std::lock_guard<std::mutex> lock(registry_mutex_);
        registry_.clear();
    }

    std::cout << "Left current room" << std::endl;
}

void Client::restart_session() {
    // Send PlayerLeave to properly close the current session on server
    if (connected_.load()) {
        rtype::net::MessageSerializer serializer;
        rtype::net::PlayerLeaveData leave_data(player_id_);
        rtype::net::Packet leave_packet = serializer.serialize_player_leave(leave_data);
        std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(leave_packet);
        udp_client_->send(packet_data);
    }

    // Wait for server to process the leave
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Full reconnect - recreate socket to clear all buffers
    reconnect();

    std::cout << "Session reset for restart" << std::endl;
}

void Client::send_heartbeat() {
    if (!connected_.load())
        return;

    auto current_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_ping_time_);

    if (elapsed >= HEARTBEAT_INTERVAL) {
        rtype::net::MessageSerializer serializer;
        uint64_t timestamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        rtype::net::PingPongData ping_data(timestamp);
        rtype::net::Packet ping_packet = serializer.serialize_ping(ping_data);
        std::vector<uint8_t> packet_data = rtype::net::ProtocolAdapter().serialize(ping_packet);
        udp_client_->send(packet_data);
        last_ping_time_ = current_time;
    }
}

void Client::update(double dt) {
    send_heartbeat();
    audio_system_.update(registry_, dt);
    network_system_.update(registry_, registry_mutex_);

    // Update ECS systems
    {
        std::lock_guard<std::mutex> lock(registry_mutex_);
        system_manager_.update(registry_, dt);
    }

    {
        std::lock_guard<std::mutex> lock(registry_mutex_);
        auto interp_view = registry_.view<rtype::ecs::component::NetworkInterpolation, rtype::ecs::component::Position,
                                          rtype::ecs::component::Velocity>();
        auto now = std::chrono::steady_clock::now();
        constexpr auto timeout = std::chrono::milliseconds(500);

        for (auto entity : interp_view) {
            GameEngine::entity_t entity_id = static_cast<GameEngine::entity_t>(entity);
            auto& interp = registry_.getComponent<rtype::ecs::component::NetworkInterpolation>(entity_id);
            auto& pos = registry_.getComponent<rtype::ecs::component::Position>(entity_id);
            auto& vel = registry_.getComponent<rtype::ecs::component::Velocity>(entity_id);

            auto time_since_update =
                std::chrono::duration_cast<std::chrono::milliseconds>(now - interp.last_update_time);

            if (time_since_update > timeout) {
                pos.x = interp.target_x;
                pos.y = interp.target_y;
                vel.vx = 0.0f;
                vel.vy = 0.0f;
            } else {
                float lerp_factor = std::min(1.0f, interp.interpolation_speed * static_cast<float>(dt));
                pos.x = pos.x + (interp.target_x - pos.x) * lerp_factor;
                pos.y = pos.y + (interp.target_y - pos.y) * lerp_factor;
                vel.vx = interp.target_vx;
                vel.vy = interp.target_vy;
            }
        }
    }

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

    {
        std::vector<GameEngine::entity_t> to_destroy;
        auto pos_view = registry_.view<rtype::ecs::component::Position>();
        for (auto entity : pos_view) {
            GameEngine::entity_t entity_id = static_cast<GameEngine::entity_t>(entity);
            bool is_player = false;
            if (registry_.hasComponent<rtype::ecs::component::Tag>(entity_id)) {
                auto& tag = registry_.getComponent<rtype::ecs::component::Tag>(entity_id);
                if (tag.name == "Player") {
                    is_player = true;
                }
            }
            if (!is_player) {
                auto& pos = registry_.getComponent<rtype::ecs::component::Position>(entity_id);
                if (pos.x < -1000.0f || pos.x > 3000.0f || pos.y < -1000.0f || pos.y > 2000.0f) {
                    to_destroy.push_back(entity_id);
                }
            }
        }
        for (auto entity : to_destroy) {
            registry_.destroyEntity(entity);
        }
    }
}

} // namespace rtype::client
