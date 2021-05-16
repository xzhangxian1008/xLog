#include "xLog.h"
#include "xLog_imple.h"

namespace xLog {

inline void SetLogFile(const char *log_file) {
    XLog::SetOutputFile(log_file);
}

inline void SetLogLevel(LogLevel log_level) {
    XLog::SetLogLevel(log_level);
}

inline LogLevel GetLogLevel() {
    XLog::GetLogLevel();
}

} // namespace xLog
