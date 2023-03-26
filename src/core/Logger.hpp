#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <atomic>
#include <chrono>

namespace core {

enum class LogLevel {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static void SetLevel(LogLevel level);
    static LogLevel GetLevel();
    static void Log(LogLevel level, const std::string& prefix, const std::string& msg);

private:
    static std::atomic<LogLevel> current_level_;
};

#define LOG_DEBUG(msg) core::Logger::Log(core::LogLevel::DEBUG, "[DEBUG]", msg)
#define LOG_INFO(msg)  core::Logger::Log(core::LogLevel::INFO,  "[INFO] ", msg)
#define LOG_WARN(msg)  core::Logger::Log(core::LogLevel::WARN,  "[WARN] ", msg)
#define LOG_ERROR(msg) core::Logger::Log(core::LogLevel::ERROR, "[ERROR]", msg)

} // namespace core

#endif // LOGGER_HPP
