#pragma once
#include <iostream>
#include <mutex>
#include <string>
#include <chrono>
#include <ctime>

class Logger {
    public:
        Logger() = default;
        static Logger& instance() {
            static Logger inst;
            return inst;
        }
        void info(const std::string& msg) {
            log("INFO", "\033[36m", msg);
        }
        void warn(const std::string& msg) {
            log("WARN", "\033[33m", msg);
        }
        void error(const std::string& msg) {
            log("ERROR", "\033[31m", msg);
        }
        void debug(const std::string& msg) {
            log("DEBUG", "\033[35m", msg);
        }
        void custom(const std::string& level, const std::string& color, const std::string& msg) {
            log(level, color, msg);
        }

    private:
        std::mutex _mutex;

        void log(const std::string& level, const std::string color, const std::string& msg) {
            std::lock_guard<std::mutex> lock(_mutex);
            std::cout 
                << color
                << "[" << timestamp() << "] "
                << "[" << level << "] "
                << msg
                << "\033[0m"
                << std::endl;
        }
        std::string timestamp() {
            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);
            return std::string(std::ctime(&t)).substr(0, 24);
        }
};
