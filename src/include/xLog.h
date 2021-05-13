#pragma once

#include <stdint.h>
#include <string>

// So, user can only include only one header file
#include "xLog_middle.h"

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

} // namespace xLog