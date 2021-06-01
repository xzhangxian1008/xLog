#include <string.h>

#include "xLog_imple.h"
#include "util.h"

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
            if (IsIOFail(xlog_singleton_.file_io_)) {
                fprintf(stderr, "xLog: write log fail in background thread\n");
                exit(-1);
            }
            xlog_singleton_.output_bf_offset_ = 0;
        } else if (xlog_singleton_.is_input_buf_full_) {
            xlog_singleton_.SwapBuffer();
            xlog_singleton_.is_input_buf_full_ = false;
            xlog_singleton_.input_bf_offset_ = 0;
            xlog_singleton_.buffer_full_cv_.notify_all();
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

void XLog::Flush() {
    std::lock_guard lg(xlog_singleton_.lock_);

    // flush input buffer
    if (xlog_singleton_.input_bf_offset_ > 0) {
        xlog_singleton_.input_buffer_[xlog_singleton_.input_bf_offset_] = 0;
        xlog_singleton_.file_io_.write(xlog_singleton_.input_buffer_, xlog_singleton_.input_bf_offset_);
        xlog_singleton_.input_bf_offset_ = 0;
        xlog_singleton_.is_input_buf_full_ = false;
    }

    // flush output buffer
    if (xlog_singleton_.output_bf_offset_ > 0) {
        xlog_singleton_.file_io_.write(xlog_singleton_.output_buffer_, xlog_singleton_.output_bf_offset_);
        xlog_singleton_.output_bf_offset_ = 0;
    }
    
    if (IsIOFail(xlog_singleton_.file_io_)) {
        fprintf(stderr, "xLog: Flush io fail\n");
        exit(-1);
    }

    // write content to disk
    xlog_singleton_.file_io_.flush();
}

} // namespace xLog
