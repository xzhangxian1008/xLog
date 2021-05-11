#include "xLog.h"
#include "xLog_imple.h"

namespace xLog {

inline void SetLogFile(const std::string &file) {
    XLog::SetLogFile(file);
}

inline void SetLogLevel(LogLevel log_level) {
    XLog::SetLogLevel(log_level);
}

inline LogLevel GetLogLevel() {
    XLog::GetLogLevel();
}

inline void Log(LogLevel log_level, const std::string &info) {
    XLog::Log(log_level, info);
}

} // namespace xLog
