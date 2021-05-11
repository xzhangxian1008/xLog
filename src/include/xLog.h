#pragma once

#include <stdint.h>
#include <string>

namespace xLog {

enum LogLevel : uint8_t {
        SILENT_LOG_LEVEL = 0,
        ERROR,
        WARNING,
        NOTICE,
        DEBUG,
        NUM_LOG_LEVELS // must be the last element in the enum
};

void SetLogFile(const std::string &file);

void SetLogLevel(LogLevel log_level);

LogLevel GetLogLevel();

void Log(LogLevel log_level, const std::string &info);

} // namespace xLog