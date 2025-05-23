#ifndef LOGGER
#define LOGGER
#include <iostream>
#include <string>
#include <mutex>
#include "global_defines.hpp"

enum class LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_NO_MESSAGE,
};

#define LOG_DEBUG(msg) Logger::get_instance().log(msg, LogLevel::LOG_DEBUG)
#define LOG_INFO(msg) Logger::get_instance().log(msg, LogLevel::LOG_INFO)
#define SET_LOG_LEVEL(level) Logger::get_instance().set_log_level(level)

class Logger {
public:
    static Logger& get_instance() {
        static Logger instance;
        return instance;
    }
    void log(const std::string& message, LogLevel level = LogLevel::LOG_INFO) {
        if (level >= log_level) {
            std::lock_guard<std::mutex> lock(mtx);
            std::string color;
            switch (level) {
                case LogLevel::LOG_DEBUG: color = "\033[34m"; break;
                case LogLevel::LOG_INFO: color = "\033[32m"; break;
                case LogLevel::LOG_NO_MESSAGE: color = "\033[0m"; break;
            }
            std::cout << color << "[" << get_level_name(level) << "] " << message << "\033[0m" << std::endl;
        }
    }

    void set_log_level(LogLevel level) {
        log_level = level;
    }

private:
    LogLevel log_level;
    std::mutex mtx;

    std::string get_level_name(LogLevel level) {
        switch (level) {
            case LogLevel::LOG_DEBUG: return "DEBUG";
            case LogLevel::LOG_INFO: return "INFO";
            default: return "UNKNOWN";
        }
    }

    Logger(LogLevel level = LogLevel::LOG_INFO) : log_level(level) {};
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif