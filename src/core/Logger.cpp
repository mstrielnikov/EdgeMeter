#include "Logger.hpp"

namespace core {

std::atomic<LogLevel> Logger::current_level_{LogLevel::INFO};

void Logger::SetLevel(LogLevel level) {
    current_level_ = level;
}

LogLevel Logger::GetLevel() {
    return current_level_.load();
}

void Logger::Log(LogLevel level, const std::string& prefix, const std::string& msg) {
    if (level >= current_level_.load()) {
        if (level == LogLevel::ERROR) {
            std::cerr << prefix << " " << msg << std::endl;
        } else {
            std::cout << prefix << " " << msg << std::endl;
        }
    }
}

} // namespace core
