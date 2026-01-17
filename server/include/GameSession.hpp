#pragma once

#include "BroadcastSystem.hpp"
#include "ClientInfo.hpp"
#include "UdpServer.hpp"
#include "Registry.hpp"
#include "SystemManager.hpp"
#include "interfaces/network/IMessageSerializer.hpp"
#include "interfaces/network/IProtocolAdapter.hpp"
#include "net/Packet.hpp"
#include "utils/GameRules.hpp"
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_set>

namespace rtype::server {

class GameSession {
  public:
    GameSession(uint32_t session_id, UdpServer& udp_server, rtype::net::IProtocolAdapter& protocol_adapter,
                rtype::net::IMessageSerializer& message_serializer);
    ~GameSession();

    uint32_t id() const {
        return session_id_;
    }

    void start();
    void stop();
    bool is_running() const {
        return running_.load();
    }

    bool handle_player_join(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_packet(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);

    void set_client_unmap_callback(std::function<void(const std::string&)> cb) {
        on_client_unmapped_ = std::move(cb);
    }

    void set_session_empty_callback(std::function<void(uint32_t)> cb) {
        on_session_empty_ = std::move(cb);
    }

    void set_game_rules(const rtype::config::GameRules& rules) {
        game_rules_ = rules;
    }

    const rtype::config::GameRules& get_game_rules() const {
        return game_rules_;
    }

    size_t client_count() const;

  private:
    void game_loop();

    void handle_player_name(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_player_move(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_player_shoot(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_game_start(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_map_resize(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_chat_message(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);

    void broadcast_message(const std::vector<uint8_t>& data, const std::string& exclude_ip = "",
                           uint16_t exclude_port = 0);
    void broadcast_to_all_clients(const std::vector<uint8_t>& data);
    void broadcast_entity_destroy(uint32_t entity_id, uint8_t reason);
    void broadcast_projectile_spawns();

    void check_client_timeouts();
    void disconnect_client(const std::string& client_key, const ClientInfo& client);

    void send_existing_entities_to_client(const std::string& client_ip, uint16_t client_port);
    void send_entity_spawn(const std::string& ip, uint16_t port, uint32_t entity_id, uint16_t entity_type,
                           uint16_t sub_type, float x, float y, float vx, float vy);
    GameEngine::entity_t create_player_entity(uint32_t player_id, const std::string& player_name);
    uint16_t get_monster_subtype(const std::string& tag_name);
    uint16_t get_projectile_subtype(const std::string& tag_name);

    uint32_t session_id_;
    UdpServer& udp_server_;
    rtype::net::IProtocolAdapter& protocol_adapter_;
    rtype::net::IMessageSerializer& message_serializer_;

    GameEngine::Registry registry_;
    GameEngine::SystemManager system_manager_;
    std::unique_ptr<BroadcastSystem> broadcast_system_;

    std::map<std::string, ClientInfo> clients_;
    mutable std::mutex clients_mutex_;
    mutable std::mutex registry_mutex_;

    uint32_t next_player_id_;
    std::atomic<bool> running_;
    std::atomic<bool> game_started_;
    std::atomic<bool> game_over_;
    std::thread game_thread_;

    std::function<void(const std::string&)> on_client_unmapped_;
    std::function<void(uint32_t)> on_session_empty_;

    std::chrono::steady_clock::time_point last_activity_;
    std::chrono::steady_clock::time_point empty_since_;

    rtype::config::GameRules game_rules_;

    static constexpr double TARGET_TICK_RATE = 60.0;
    static constexpr std::chrono::duration<double> TICK_DURATION =
        std::chrono::duration<double>(1.0 / TARGET_TICK_RATE);
    static constexpr std::chrono::seconds CLIENT_TIMEOUT_DURATION = std::chrono::seconds(10);
    static constexpr std::chrono::seconds EMPTY_TIMEOUT_DURATION = std::chrono::seconds(10);
};

} // namespace rtype::server
