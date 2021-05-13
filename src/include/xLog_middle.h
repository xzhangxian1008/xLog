#pragma once

#include "xLog_imple.h"

namespace xLog {

// only support %d and %s
enum : uint8_t {
    INTEGER = 0,
    STRING
};



template<typename... Types, int M>
void Log(LogLevel log_level, const char *file_name, int line, char (&format)[M] , const Types&... args) {
    if (log_level > XLog::GetLogLevel()) {
        return;
    }


}

#define X_LOG(log_level, format, ...) do {\
    Log(__FILE__, __LINE__, format, ##__VA_ARGS__);\
} while(0);

} // namespace xLog
