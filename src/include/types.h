#pragma once

namespace xLog {

enum LogLevel {
        SILENT_LOG_LEVEL = 0,
        ERROR,
        WARNING,
        NOTICE,
        DEBUG,
        NUM_LOG_LEVELS // must be the last element in the enum
};

} // namespace xLog
