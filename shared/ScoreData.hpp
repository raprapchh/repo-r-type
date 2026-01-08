#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace rtype::score {

/// @brief Represents a single score entry
struct ScoreEntry {
    std::string player_name;
    uint32_t score;
    uint64_t timestamp; // Unix timestamp
    
    ScoreEntry(const std::string& name = "", uint32_t s = 0, uint64_t ts = 0)
        : player_name(name), score(s), timestamp(ts) {}
    
    bool operator>(const ScoreEntry& other) const {
        return score > other.score;
    }
};

/// @brief Container for all scores (solo and multiplayer)
struct ScoreboardData {
    std::vector<ScoreEntry> solo_scores;
    std::vector<ScoreEntry> multi_scores;
    
    static constexpr size_t MAX_SCORES = 10;
    
    void add_solo_score(const std::string& player_name, uint32_t score, uint64_t timestamp);
    void add_multi_score(const std::string& player_name, uint32_t score, uint64_t timestamp);
    
private:
    void add_and_sort(std::vector<ScoreEntry>& scores, const std::string& player_name, 
                      uint32_t score, uint64_t timestamp);
};

} // namespace rtype::score
