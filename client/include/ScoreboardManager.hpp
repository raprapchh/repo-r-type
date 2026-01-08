#pragma once

#include "../../shared/ScoreData.hpp"
#include <string>
#include <memory>

namespace rtype::client {

/// @brief Manages loading and saving scoreboard data to disk
class ScoreboardManager {
  public:
    ScoreboardManager();
    ~ScoreboardManager() = default;

    /// @brief Load scoreboard from file
    /// @return true if loaded successfully, false otherwise
    bool load();

    /// @brief Save scoreboard to file
    /// @return true if saved successfully, false otherwise
    bool save();

    /// @brief Add a score to the solo scoreboard
    void add_solo_score(const std::string& player_name, uint32_t score);

    /// @brief Add a score to the multiplayer scoreboard
    void add_multi_score(const std::string& player_name, uint32_t score);

    /// @brief Get the scoreboard data
    const rtype::score::ScoreboardData& get_data() const {
        return data_;
    }

    /// @brief Clear all scores (useful for testing)
    void clear_all_scores();

  private:
    rtype::score::ScoreboardData data_;
    std::string save_file_path_;

    uint64_t get_current_timestamp() const;
    std::string get_save_directory() const;
};

} // namespace rtype::client
