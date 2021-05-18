#pragma once

#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <fstream>
#include <atomic>
#include <chrono>

#include "config.h"
#include "types.h"

namespace xLog {

class XLog {
public:
    ~XLog() {
        if (!file_io_.is_open()) {
            file_io_.close();
        }

        is_running_ = false;

        if (background_thread_.joinable()) {
            background_thread_.join();
        }

        delete[] input_buffer_;
        delete[] output_buffer_;
    }

    XLog(const XLog &) = delete;

    XLog& operator=(const XLog &) = delete;

    static XLog* GetInstance() {
        return &xlog_singleton_;
    }

    static void SetOutputFile(const char *file) {
        xlog_singleton_.file_io_.open(std::string(file), std::ios::in);
        if (xlog_singleton_.file_io_.is_open()) {
            // the file has existed
            return;
        }

        // create the file
        std::ios_base::openmode om = std::ios::in | std::ios::out | std::ios::trunc;
        xlog_singleton_.file_io_.open(file, om);
        if (!xlog_singleton_.file_io_.is_open()) {
            std::string err("Unable to create the log file:");
            err.append(file);
            throw std::ios_base::failure(err);
        }
    }

    static void SetLogLevel(LogLevel log_level) {
        std::lock_guard(xlog_singleton_.lock_);
        xlog_singleton_.log_level_ = log_level;
    }

    static inline LogLevel GetLogLevel() {
        std::lock_guard(xlog_singleton_.lock_);
        return xlog_singleton_.log_level_;
    }

    static void Log(const char* log_store, uint32_t log_size);

private:
    XLog();

    // no lock function, ensure it can be called with lock's protection
    void SwapBuffer();

    // background thread will continously run in this function
    static void Background();

    std::mutex lock_;

    std::fstream file_io_;

    // the output file, must be set
    std::string log_file_;

    static XLog xlog_singleton_;

    // logging threads will notify background thread when the buffer is full
    std::condition_variable buffer_full_cv_;

    // logging threads will put data in this buffer
    char *input_buffer_;

    // point to the head of the empty space
    // input_bf_offset_ == 0 means that the buffer is empty, BUFFER_SIZE means full
    uint32_t input_bf_offset_;

    bool is_input_buf_full_;

    // background thread will persist data in this buffer to the disk
    char *output_buffer_;

    // similiar to the input_bf_offset_
    uint32_t output_bf_offset_;

    std::thread background_thread_;

    std::atomic_bool is_running_;

    LogLevel log_level_;
};

} // namespace xLog
