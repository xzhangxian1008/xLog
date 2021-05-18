#pragma once

#include <string>
#include <deque>
#include <utility>
#include <string.h>

#include "xLog_imple.h"

namespace xLog {

// only support %d and %s
enum Type : uint8_t {
    INTEGER = 0,
    STRING
};

uint32_t GetArgs(std::deque<std::string>& args_strs) {
    return 0;
}

/**
 * get size of all the arguments and convert them to string
 */
template<typename T, typename... Types>
uint32_t GetArgs(
    std::deque<std::string>& args_strs,
    const T& p,
    const Types... args)
{
    if (typeid(p) == typeid(int)) {
        args_strs.push_back(std::to_string(p));
    } else if (typeid(p) == typeid(char*)) {
        args_strs.push_back(std::string(p));
    } else {
        // TODO Maybe we can handle this exception more elegantly
        fprintf(stderr, "xLOG: Not support %s", typeid(p).name());
        exit(-1);
    }

    return args_strs.back().size() + GetArgsSize(args_strs, args...);
}

/**
 * @return a pair with number of placeholder and the format's length(include placeholders)
 */
std::pair<uint32_t, uint32_t>
ProcessFormat(const char *format) {
    uint32_t idx = 0;
    uint32_t num = 0;

    while (format[idx] != 0) {
        if (format[idx] != '%' &&
            (format[idx] != 'd' || format[idx] != 's')) {
            idx++;
            continue;
        }

        num++;
        idx += 2;
    }

    return std::pair<uint32_t, uint32_t>(num, idx);
}

uint32_t GetLogTotalSize(
    LogLevel log_level,
    const char *file_name,
    int line,
    uint32_t format_size,
    uint32_t args_size)
{
    uint32_t total_size = format_size + args_size + EXTRA_SIZE;

    switch (log_level) {
        case LogLevel::ERROR:
            total_size += 7; // [ERROR]
            break;
        case LogLevel::WARNING:
            total_size += 9; // [WARNING]
            break;
        case LogLevel::NOTICE:
            total_size += 8; // [NOTICE]
            break;
        case LogLevel::DEBUG:
            total_size += 7; // [DEBUG]
            break;
        default:
            break;
    }

    uint32_t file_name_size = 0;
    while (file_name[file_name_size] != 0) {
        file_name_size++;
    }
    total_size += file_name_size;

    return total_size + std::to_string(line).size();
}

void Log(
    LogLevel log_level,
    const char* file_name,
    int line,
    const char* format,
    std::deque<std::string>& args_str,
    char* log_store,
    uint32_t log_size)
{
    // Load all the log data into memory.
    uint32_t idx = 0;
    switch (log_level) {
        case LogLevel::ERROR:
            strcpy(log_store + idx, "[ERROR] ");
            idx += 8;
            break;
        case LogLevel::WARNING:
             strcpy(log_store + idx, "[WARNING] ");
             idx += 10;
            break;
        case LogLevel::NOTICE:
             strcpy(log_store + idx, "[NOTICE] ");
             idx += 9;
            break;
        case LogLevel::DEBUG:
             strcpy(log_store + idx, "[DEBUG] ");
             idx += 8;
            break;
        default:
            break;
    }

    uint32_t file_idx = 0;
    while (file_name[file_idx] != 0) {
        log_store[idx++] = file_name[file_idx];
        file_idx++;
    }
    log_store[idx++] = ':';
    log_store[idx++] = ' ';

    std::string num_str(std::to_string(line));
    strcpy(log_store + idx, num_str.c_str());
    idx += num_str.size();
    log_store[idx++] = ':';
    log_store[idx++] = ' ';

    uint32_t format_idx = 0;
    while (format[format_idx] != 0) {
        if (format[format_idx] != '%' &&
            (format[format_idx] != 'd' || format[format_idx] != 's')) {
            log_store[idx++] = format[format_idx];
            format_idx++;
            continue;
        }

        strcpy(log_store + idx, args_str.front().c_str());
        idx += args_str.front().size();
        args_str.pop_front();
        format_idx += 2;
    }

    // Now, we have load all the log data into memory.
    XLog::Log(log_store, log_size);
}

/**
 * Try our best to avoid calling malloc function, 
 * so that we can speed up the acquisition of the memory.
 * 
 * Log format example: [DEBUG] /home/xzx/main.cpp: 345: ...content...
 */
template<typename... Types>
inline void x_log(
    LogLevel log_level,
    const char *file_name,
    int line,
    const char *format,
    const Types&... args)
{
    if (log_level > XLog::GetLogLevel()) {
        return;
    }

    std::pair<uint32_t, uint32_t> ret = ProcessFormat(format);
    if (ret.first != sizeof...(args)) {
        fprintf(stderr, "xLog: number of placeholder and argument is inconsistent");
        exit(-1);
    }

    std::deque<std::string> args_str;
    uint32_t args_size = GetArgs(args_str, args...);
    uint32_t log_total_size = GetLogTotalSize(log_level, file_name, line, ret.second - 2*ret.first, args_size);
    char *log_store;
    
    // If size of the log is bigger than 500, then we get memory from malloc function.
    // This local memory will always be used until data has been put into the input buffer.
    char local_mem[LOCAL_MEM_SIZE + 1];
    log_store = local_mem;

    if (log_total_size > LOCAL_MEM_SIZE) {
        log_store = new char[LOCAL_MEM_SIZE+1];
    }

    Log(log_level, file_name, line, format, args_str, log_store, log_total_size);

    if (log_total_size > LOCAL_MEM_SIZE) {
        delete[] log_store;
    }
}

#define X_LOG(log_level, format, ...) do {\
    x_log(log_level, __FILE__, __LINE__, format, ##__VA_ARGS__);\
} while(0);

} // namespace xLog
