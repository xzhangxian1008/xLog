#include <string.h>

#include "xLog_imple.h"

namespace xLog {

XLog XLog::xlog_singleton_;

XLog::XLog() : 
    background_thread_(Background),
    is_running_(true),
    is_input_buf_full_(false),
    input_bf_offset_(0),
    output_bf_offset_(0),
    log_level_(LogLevel::DEBUG)
{
    input_buffer_ = new char[BUFFER_SIZE];
    output_buffer_ = new char[BUFFER_SIZE];
}

void XLog::Background() {
    while (xlog_singleton_.is_running_) {
        std::unique_lock<std::mutex> ul(xlog_singleton_.lock_);
        xlog_singleton_.buffer_full_cv_.wait_for(ul, std::chrono::milliseconds(100));

        if (xlog_singleton_.output_bf_offset_ > 0) {
            ul.unlock();
            xlog_singleton_.file_io_.write(xlog_singleton_.output_buffer_, xlog_singleton_.output_bf_offset_);
            if (xlog_singleton_.file_io_.fail()) {
                fprintf(stderr, "Error at write(): %s\n", strerror(errno));
            }
            xlog_singleton_.file_io_.flush();
        } else if (xlog_singleton_.is_input_buf_full_) {
            xlog_singleton_.SwapBuffer();
        }
    }
}

void XLog::SwapBuffer() {
    char *tmp_str;
    tmp_str = input_buffer_;
    input_buffer_ = output_buffer_;
    output_buffer_ = tmp_str;

    uint32_t tmp_offset;
    tmp_offset = input_bf_offset_;
    input_bf_offset_ = output_bf_offset_;
    output_bf_offset_ = tmp_offset;
}

void XLog::Log(const char* log_store, uint32_t log_size) {
    std::unique_lock<std::mutex> ul(xlog_singleton_.lock_);

    if (xlog_singleton_.is_input_buf_full_) {
        xlog_singleton_.buffer_full_cv_.wait(ul, [&] { return !xlog_singleton_.is_input_buf_full_; });
    }

    // We need a loop here because the thread may be unlucky to fail to
    // get the lock for a long time and the buffer may have been loaded
    // a lot of data during the wait.
    while (BUFFER_SIZE - xlog_singleton_.input_bf_offset_ < log_size) {
        xlog_singleton_.is_input_buf_full_ = true;
        xlog_singleton_.buffer_full_cv_.notify_all(); // ensure the background thread could be notified
        xlog_singleton_.buffer_full_cv_.wait(ul, [&] { return !xlog_singleton_.is_input_buf_full_; });
    }

    memcpy(xlog_singleton_.input_buffer_ + xlog_singleton_.input_bf_offset_, log_store, log_size);
    xlog_singleton_.input_bf_offset_ += log_size;
}

} // namespace xLog
