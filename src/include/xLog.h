#pragma once

#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "config.h"

namespace xLog {

class XLog {
public:
    ~XLog() {
        // TODO release some resource
    }

    XLog(const XLog &) = delete;

    XLog& operator=(const XLog &) = delete;

    static XLog* GetInstance() {
        return &xlog_singleton_;
    }

    void SetOutputFile(std::string file) {
        
    }

private:
    XLog() {
        background_thread_ = std::thread(Background);
    }

    void Background();

    // the output file, must be set
    std::string log_file_;

    static XLog xlog_singleton_;

    std::mutex lock_;

    // logging threads will notify background thread when the buffer is full
    std::condition_variable buffer_full_cv_;

    // logging threads will put data in this buffer
    char input_buffer_[BUFFER_SIZE];

    // background thread will persist data in this buffer to the disk
    char output_buffer_[BUFFER_SIZE];

    std::thread background_thread_;
};

} // namespace xLog