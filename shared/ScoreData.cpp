#include "ScoreData.hpp"
#include <algorithm>

namespace rtype::score {

void ScoreboardData::add_solo_score(const std::string& player_name, uint32_t score, uint64_t timestamp) {
    add_and_sort(solo_scores, player_name, score, timestamp);
}

void ScoreboardData::add_multi_score(const std::string& player_name, uint32_t score, uint64_t timestamp) {
    add_and_sort(multi_scores, player_name, score, timestamp);
}

void ScoreboardData::add_and_sort(std::vector<ScoreEntry>& scores, const std::string& player_name, uint32_t score,
                                  uint64_t timestamp) {
    scores.emplace_back(player_name, score, timestamp);

    // Sort in descending order
    std::sort(scores.begin(), scores.end(), std::greater<ScoreEntry>());

    // Keep only top scores
    if (scores.size() > MAX_SCORES) {
        scores.resize(MAX_SCORES);
    }
}

} // namespace rtype::score
