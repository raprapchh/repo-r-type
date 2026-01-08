#include "../include/ScoreboardManager.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <sstream>

namespace rtype::client {

ScoreboardManager::ScoreboardManager() {
    save_file_path_ = get_save_directory() + "/scoreboard.txt";
}

std::string ScoreboardManager::get_save_directory() const {
    return "client/saves";
}

uint64_t ScoreboardManager::get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

bool ScoreboardManager::load() {
    std::ifstream file(save_file_path_);
    if (!file.is_open()) {
        std::cerr << "Scoreboard file not found. Starting with empty scoreboard." << std::endl;
        return false;
    }

    data_.solo_scores.clear();
    data_.multi_scores.clear();

    std::string line;
    std::string current_section;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line == "[SOLO]") {
            current_section = "SOLO";
            continue;
        } else if (line == "[MULTI]") {
            current_section = "MULTI";
            continue;
        }

        // Parse score line: "PlayerName:12345:1234567890"
        std::istringstream iss(line);
        std::string name;
        uint32_t score;
        uint64_t timestamp;
        char delim1;

        if (std::getline(iss, name, ':') && 
            (iss >> score) && (iss >> delim1) && 
            (iss >> timestamp)) {
            
            if (current_section == "SOLO") {
                data_.solo_scores.emplace_back(name, score, timestamp);
            } else if (current_section == "MULTI") {
                data_.multi_scores.emplace_back(name, score, timestamp);
            }
        }
    }

    file.close();
    std::cout << "Loaded " << data_.solo_scores.size() << " solo scores and " 
              << data_.multi_scores.size() << " multi scores." << std::endl;
    return true;
}

bool ScoreboardManager::save() {
    // Create directory if it doesn't exist
    std::filesystem::create_directories(get_save_directory());

    std::ofstream file(save_file_path_);
    if (!file.is_open()) {
        std::cerr << "Failed to open scoreboard file for writing: " << save_file_path_ << std::endl;
        return false;
    }

    file << "# R-Type Scoreboard Save File\n";
    file << "# Format: PlayerName:Score:Timestamp\n\n";

    file << "[SOLO]\n";
    for (const auto& entry : data_.solo_scores) {
        file << entry.player_name << ":" << entry.score << ":" << entry.timestamp << "\n";
    }

    file << "\n[MULTI]\n";
    for (const auto& entry : data_.multi_scores) {
        file << entry.player_name << ":" << entry.score << ":" << entry.timestamp << "\n";
    }

    file.close();
    std::cout << "Saved scoreboard to " << save_file_path_ << std::endl;
    return true;
}

void ScoreboardManager::add_solo_score(const std::string& player_name, uint32_t score) {
    data_.add_solo_score(player_name, score, get_current_timestamp());
    save();
}

void ScoreboardManager::add_multi_score(const std::string& player_name, uint32_t score) {
    data_.add_multi_score(player_name, score, get_current_timestamp());
    save();
}

void ScoreboardManager::clear_all_scores() {
    data_.solo_scores.clear();
    data_.multi_scores.clear();
    save();
}

} // namespace rtype::client
