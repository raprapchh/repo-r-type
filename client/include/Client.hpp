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
#include "../shared/net/Protocol.hpp"
#include "../shared/interfaces/network/IProtocolAdapter.hpp"
#include "../shared/interfaces/network/IMessageSerializer.hpp"
#include "NetworkSystem.hpp"
#include "UdpClient.hpp"
#include "Renderer.hpp"
// #include "../../ecs/include/systems/AudioSystem.hpp"

// rtype::ecs::AudioSystem audio_system_;

namespace rtype::client {

/// @brief Main game client handling network and game state
class Client {
  public:
    Client(const std::string& host, uint16_t port, Renderer& renderer);
    ~Client();

    void connect();
    void disconnect();
    void run();
    void update(double dt);

    void send_move(float vx, float vy);
    void send_shoot(int32_t x, int32_t y, int chargeLevel = 0);

    bool is_connected() const {
        return connected_.load();
    }

    GameEngine::Registry& get_registry() {
        return registry_;
    }

    std::mutex& get_registry_mutex() {
        return registry_mutex_;
    }

  public:
    void set_player_join_callback(std::function<void(uint32_t, const std::string&)> callback);
    void set_player_name_callback(std::function<void(uint32_t, const std::string&)> callback);
    void set_game_start_callback(std::function<void()> callback);

    void send_game_start_request();
    void send_player_name_update(const std::string& name);

    uint32_t get_player_id() const;
    std::string get_player_name() const;

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
    std::string player_name_;

    std::function<void(uint32_t, const std::string&)> player_join_callback_;
    std::function<void(uint32_t, const std::string&)> player_name_callback_;
    std::function<void()> game_start_callback_;

    // Store pending players (ID, Name)
    std::vector<std::pair<uint32_t, std::string>> pending_players_;
    GameEngine::Registry registry_;
    NetworkSystem network_system_;
    // rtype::ecs::AudioSystem audio_system_;
    std::mutex registry_mutex_;
    Renderer& renderer_;

    std::string host_;
    uint16_t port_;

    std::chrono::steady_clock::time_point last_ping_time_;
    static constexpr std::chrono::seconds HEARTBEAT_INTERVAL = std::chrono::seconds(3);
    void send_heartbeat();
};

} // namespace rtype::client
