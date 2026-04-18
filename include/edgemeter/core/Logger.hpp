#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <string_view>
#include <atomic>

namespace core {

enum class LogLevel {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR
};

// Source location mapping for internal C-macro translations.
struct source_location {
    const char* file_name_;
    int line_;
    const char* function_name_;

    static constexpr source_location current(
        const char* file = __builtin_FILE(),
        int line = __builtin_LINE(),
        const char* func = __builtin_FUNCTION()
    ) noexcept {
        return {file, line, func};
    }

    constexpr const char* file_name() const noexcept { return file_name_; }
    constexpr int line() const noexcept { return line_; }
    constexpr const char* function_name() const noexcept { return function_name_; }
};

class Logger {
public:
    static void SetLevel(LogLevel level);
    static LogLevel GetLevel();

    static void Debug(const std::string& msg, const source_location& loc = source_location::current());
    static void Info(const std::string& msg, const source_location& loc = source_location::current());
    static void Warn(const std::string& msg, const source_location& loc = source_location::current());
    static void Error(const std::string& msg, const source_location& loc = source_location::current());

private:
    static void LogFull(LogLevel level, std::string_view prefix, const std::string& msg, const source_location& loc);
    static std::atomic<LogLevel> current_level_;
};

} // namespace core

#endif // LOGGER_HPP
