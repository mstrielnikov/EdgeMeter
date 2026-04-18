#include <edgemeter/core/Logger.hpp>
#include <iostream>

namespace core {

std::atomic<LogLevel> Logger::current_level_{LogLevel::INFO};

void Logger::SetLevel(LogLevel level) {
    current_level_ = level;
}

LogLevel Logger::GetLevel() {
    return current_level_.load();
}

void Logger::LogFull(LogLevel level, std::string_view prefix, const std::string& msg, const source_location& loc) {
    if (level >= current_level_.load()) {
        if (level == LogLevel::ERROR) {
            std::cerr << prefix << " [" << loc.file_name() << ":" << loc.line() << "] " << msg << "\n";
        } else {
            std::cout << prefix << " [" << loc.file_name() << ":" << loc.line() << "] " << msg << "\n";
        }
    }
}

void Logger::Debug(const std::string& msg, const source_location& loc) { LogFull(LogLevel::DEBUG, "[DEBUG]", msg, loc); }
void Logger::Info(const std::string& msg, const source_location& loc)  { LogFull(LogLevel::INFO,  "[INFO] ", msg, loc); }
void Logger::Warn(const std::string& msg, const source_location& loc)  { LogFull(LogLevel::WARN,  "[WARN] ", msg, loc); }
void Logger::Error(const std::string& msg, const source_location& loc) { LogFull(LogLevel::ERROR, "[ERROR]", msg, loc); }

} // namespace core
