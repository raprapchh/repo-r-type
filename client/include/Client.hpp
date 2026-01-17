#pragma once

#include <asio.hpp>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "net/Protocol.hpp"
#include "interfaces/network/IProtocolAdapter.hpp"
#include "interfaces/network/IMessageSerializer.hpp"
#include "NetworkSystem.hpp"
#include "UdpClient.hpp"
#include "Renderer.hpp"
#include "ScoreboardManager.hpp"
#include "systems/AudioSystem.hpp"
#include "SystemManager.hpp"
#include "utils/GameRules.hpp"

namespace rtype::client {

/// @brief Main game client handling network and game state
class Client {
  public:
    Client(const std::string& host, uint16_t port, Renderer& renderer);
    ~Client();

    void connect();
    void disconnect();
    void reconnect();
    void run();
    void update(double dt);

    void send_move(float vx, float vy);
    void send_shoot(int32_t x, int32_t y, int chargeLevel = 0);
    void send_ping(uint64_t timestamp);

    bool is_connected() const {
        return connected_.load();
    }

    GameEngine::Registry& get_registry() {
        return registry_;
    }

    GameEngine::SystemManager& get_system_manager() {
        return system_manager_;
    }

    std::mutex& get_registry_mutex() {
        return registry_mutex_;
    }

  public:
    void set_player_join_callback(std::function<void(uint32_t, const std::string&)> callback);
    void set_player_name_callback(std::function<void(uint32_t, const std::string&)> callback);
    void set_game_start_callback(std::function<void()> callback);

    void set_session_id(uint32_t session_id) {
        session_id_ = session_id;
    }

    void send_game_start_request();
    void send_player_name_update(const std::string& name);
    void send_chat_message(const std::string& message);
    void leave_room();
    void restart_session();
    void set_offline_ids(uint32_t session_id, uint32_t player_id) {
        session_id_ = session_id;
        player_id_ = player_id;
        network_system_.set_player_id(player_id_);
    }

    void set_chat_message_callback(std::function<void(uint32_t, const std::string&, const std::string&)> callback);

    void set_room_list_callback(std::function<void(uint32_t, uint8_t, uint8_t, uint8_t, const std::string&)> callback);

    void set_lobby_update_callback(std::function<void(int8_t, int8_t)> callback);

    void request_room_list();
    void create_room(const std::string& room_name, uint8_t max_players = 4,
                     rtype::config::GameMode mode = rtype::config::GameMode::COOP,
                     rtype::config::Difficulty difficulty = rtype::config::Difficulty::NORMAL,
                     bool friendly_fire = false, uint8_t lives = 3);
    void join_room(uint32_t session_id);

    uint32_t get_player_id() const;
    std::string get_player_name() const;

    ScoreboardManager& get_scoreboard_manager() {
        return scoreboard_manager_;
    }

    rtype::ecs::AudioSystem& get_audio_system() {
        return audio_system_;
    }

  private:
    void receive_loop();
    void handle_udp_receive(const asio::error_code& error, std::size_t bytes_transferred,
                            const std::vector<uint8_t>& data);
    void handle_server_message(const std::vector<uint8_t>& data);

    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<UdpClient> udp_client_;
    std::unique_ptr<std::thread> network_thread_;
    std::unique_ptr<rtype::net::IProtocolAdapter> protocol_adapter_;
    std::unique_ptr<rtype::net::IMessageSerializer> message_serializer_;
    std::atomic<bool> connected_;
    uint32_t player_id_;
    uint32_t session_id_ = 1;
    std::string player_name_;

    std::function<void(uint32_t, const std::string&)> player_join_callback_;
    std::function<void(uint32_t, const std::string&)> player_name_callback_;
    std::function<void()> game_start_callback_;
    std::function<void(uint32_t, const std::string&, const std::string&)> chat_message_callback_;
    std::function<void(uint32_t, uint8_t, uint8_t, uint8_t, const std::string&)> room_list_callback_;
    std::function<void(int8_t, int8_t)> lobby_update_callback_;

    std::vector<std::pair<uint32_t, std::string>> pending_players_;
    GameEngine::Registry registry_;
    GameEngine::SystemManager system_manager_;
    NetworkSystem network_system_;
    rtype::ecs::AudioSystem audio_system_;
    ScoreboardManager scoreboard_manager_;
    std::mutex registry_mutex_;
    Renderer& renderer_;

    std::string host_;
    uint16_t port_;

    std::string pending_create_room_name_;

    std::chrono::steady_clock::time_point last_ping_time_;
    static constexpr std::chrono::seconds HEARTBEAT_INTERVAL = std::chrono::seconds(3);
    void send_heartbeat();
};

} // namespace rtype::client
