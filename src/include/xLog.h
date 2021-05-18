#pragma once

#include <stdint.h>
#include <string>

#include "types.h"
#include "xLog_imple.h"

namespace xLog {

inline void SetLogFile(const char *log_file) {
    XLog::SetOutputFile(log_file);
}

inline void SetLogLevel(LogLevel log_level) {
    XLog::SetLogLevel(log_level);
}

inline LogLevel GetLogLevel() {
    return XLog::GetLogLevel();
}

} // namespace xLog

// So, we can ensure the xLog.h can be at the 
// beginning of the source code after preprocessing
#include "xLog_middle.h"