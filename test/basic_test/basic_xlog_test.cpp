#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <unistd.h>
#include <thread>
#include <vector>
#include <condition_variable>
#include <chrono>

#include "gtest/gtest.h"
#include "xLog.h"
#include "util.h"

namespace xLog_basic_test {

/**
 * Only check the correctness of the logs' format
 * 
 * Log format example: [DEBUG] /home/xzx/main.cpp: 345: ...content...
 * 
 * @return true: ok, false: invalid format
 */
bool CheckFormat(const char *log) {
    int idx = 0;
    if (log[idx] != '[') {
        return false;
    }

    while (log[idx] != ']' && log[idx] != 0) idx++;

    if (log[idx] != ']') return false;
    idx++;
    if (log[idx] != ' ') return false;
    idx++;
    
    while (log[idx] != ':' && log[idx] != 0) idx++;
    if (log[idx] != ':') return false;
    idx++;
    if (log[idx] != ' ') return false;
    idx++;

    while (log[idx] != ':' && log[idx] != 0) {
        if (log[idx] < '0' || log[idx] > '9') {
            return false;
        }
        idx++;
    }
    if (log[idx] != ':') return false;
    idx++;
    if (log[idx] != ' ') return false;

    return true;
}

bool CheckLogLevel(const char *log, const char *target) {
    ssize_t target_size = 0;
    while (target[target_size] != 0) {
        target_size++;
    }
    
    char buf[target_size + 1];
    memset(buf, 0, target_size + 1);

    for (int i = 0; i < target_size; i++) {
        buf[i] = log[i];
    }
    
    return strcmp(target, buf) == 0 ? true : false;
}

bool CheckLogContent(const char *log, const char *target) {
    int content_size = 0;
    while (target[content_size] != 0) {
        content_size++;
    }

    // According to the log format, there are some log level data before the file name.
    // 100 is definitely long enough to store these data.
    char buf[content_size + 1];
    memset(buf, 0, content_size + 1);

    int content_start_idx = 0;
    int space = 3;
    while (space > 0) {
        content_start_idx++;
        if (log[content_start_idx] == ' ') {
            space--;
        }
    }
    content_start_idx++;

    for (int i = 0; i < content_size; i++) {
        buf[i] = log[content_start_idx + i];
    }

    return strcmp(target, buf) == 0 ? true : false;
}

/**
 * Data ends with '\n' will be converted to the line.
 * '\n' should be peeled off.
 * 
 * @param buf store data that need to be converted
 * @param lines container which stores the converted lines
 */
void GetLines(char *buf, int max_size, std::vector<std::string> &lines) {
    // refer to the head of a line
    int start_idx = 0;

    // refer to the next char of the end of a line
    int end_idx = 0;

    while (end_idx < max_size && buf[end_idx] != 0) {
        if (buf[end_idx] == '\n') {
            // convert data to the line
            buf[end_idx] = 0;
            lines.emplace_back(std::string(static_cast<char *>(buf + start_idx)));
            buf[end_idx] = '\n';

            start_idx = end_idx + 1;
            end_idx = start_idx;
        }

        end_idx++;
    }
}

/**
 * Logs will be converted to std::string and pushed into std::vector
 */
void GetAllLogs(std::vector<std::string> &lines, std::fstream &file_io_) {
    int read_buf_size = 100000;
    char buf[read_buf_size + 1];
    memset(buf, 0, read_buf_size + 1);

    // read all the logs and check their formats
    file_io_.seekp(0);
    file_io_.read(buf, read_buf_size);
    int read_cnt = file_io_.gcount();
    while (read_cnt > 0) {
        GetLines(buf, read_buf_size + 1, lines);

        if (buf[read_cnt - 1] != 0) {
            // Get an incomplete log, then back the cursor to the head of the log
            int back_step_num = 1;

            // NOTICE ensure read_buf_size > log size
            // Size of the log should be smaller than the
            // buf read_buf_size or it will cause some problems
            while (buf[read_cnt - back_step_num] != '\n') {
                back_step_num++;
            }
            back_step_num--;

            file_io_.seekp(-1 * back_step_num, std::ios::cur);
        }
        memset(buf, 0, read_buf_size + 1);
        file_io_.read(buf, read_buf_size);
        read_cnt = file_io_.gcount();
    }
}

/**
 * Log format example: [DEBUG] /home/xzx/main.cpp: 345: ...content...
 * 
 * Write only one log into file
 */
TEST(BasicLogTest, BasicTest) {
    const char *log_file = "basic_log_test.txt";
    remove(log_file);

    // create file
    std::fstream file_io_(log_file, std::ios::in | std::ios::out | std::ios::trunc);

    xLog::SetLogLevel(xLog::LogLevel::DEBUG);
    xLog::SetLogFile(log_file);

    const char *log1_content = "This is log1";
    const char *log1_level = "[DEBUG]";

    const char *log2_content = "This is log2";
    const char *log2_level = "[NOTICE]";

    const char *log3_content = "This is log3";
    const char *log3_level = "[WARNING]";

    const char *log4_content = "This is log4";
    const char *log4_level = "[ERROR]";

    X_LOG(xLog::DEBUG, log1_content);
    X_LOG(xLog::NOTICE, "This %s log%d", "is", 2);
    X_LOG(xLog::WARNING, "This is %s%d", "log", 3);
    X_LOG(xLog::ERROR, "%s %s %s%d", "This", "is", "log", 4);
    
    // ensure the content has been written into the file
    xLog::Flush();

    // I think it's large enough to store all the logs
    int buf_size = 10000;
    char buf[buf_size + 1];
    memset(buf, 0, buf_size + 1);
    file_io_.read(buf, buf_size);

    std::vector<std::string> lines;
    GetLines(buf, buf_size + 1, lines);
    ASSERT_EQ(lines.size(), 4);

    EXPECT_TRUE(CheckLogLevel(lines[0].c_str(), log1_level));
    EXPECT_TRUE(CheckLogContent(lines[0].c_str(), log1_content));

    EXPECT_TRUE(CheckLogLevel(lines[1].c_str(), log2_level));
    EXPECT_TRUE(CheckLogContent(lines[1].c_str(), log2_content));

    EXPECT_TRUE(CheckLogLevel(lines[2].c_str(), log3_level));
    EXPECT_TRUE(CheckLogContent(lines[2].c_str(), log3_content));

    EXPECT_TRUE(CheckLogLevel(lines[3].c_str(), log4_level));
    EXPECT_TRUE(CheckLogContent(lines[3].c_str(), log4_content));

    remove(log_file);
}

/**
 * Record a bunch of logs with a single thread
 */
TEST(BasicLogTest, SingleThreadManyLogsTest) {
    const char *log_file = "single_thd_many_logs_test.txt";
    remove(log_file);

    // create file
    std::fstream file_io_(log_file, std::ios::in | std::ios::out | std::ios::trunc);

    xLog::SetLogLevel(xLog::LogLevel::DEBUG);
    xLog::SetLogFile(log_file);

    // create a lot of logs
    size_t log_num = 1000000;
    std::string log_content("pretend to have many many info...");
    for (size_t i = 0; i < log_num; i++) {
        X_LOG(xLog::LogLevel::DEBUG, log_content.c_str());
    }

    // ensure the content has been written into the file
    xLog::Flush();

    std::vector<std::string> lines;
    GetAllLogs(lines, file_io_);

    bool ok = true;
    if (lines.size() != log_num) {
        ok = false;
    }

    for (size_t i = 0; i < log_num && ok; i++) {
        if (!CheckFormat(lines[i].c_str())) {
            ok = false;
        }
    }
    EXPECT_TRUE(ok);
    remove(log_file);
}

/**
 * Record a bunch of logs with multi-threads
 */
TEST(BasicLogTest, MultiThreadManyLogsTest) {
    const char *log_file = "multi_thd_many_logs_test.txt";
    remove(log_file);

    // create file
    std::fstream file_io_(log_file, std::ios::in | std::ios::out | std::ios::trunc);

    xLog::SetLogLevel(xLog::LogLevel::DEBUG);
    xLog::SetLogFile(log_file);

    // total number of threads
    int thread_num = 8;
    // number of threads that finish all the works
    int finish_num = 0;
    // log number that every thread should record
    int log_num = 500000;
    std::vector<std::thread> threads;
    std::condition_variable cond;
    std::mutex mt;
    std::string content("This is a log recorded by thread ");
    
    // start threads and record a bunch of logs
    for (int i = 0; i < thread_num; i++) {
        std::thread thd([&](int thd_num) {
            // wait for the completion of the creation of threads
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::string log_content(content);
            log_content.append(std::to_string(thd_num));

            for (int i = 0; i < log_num; i++) {
                X_LOG(xLog::LogLevel::DEBUG, log_content.c_str());
            }

            std::lock_guard lg(mt);
            finish_num++;
            cond.notify_one();
        }, i);
        
        threads.push_back(std::move(thd));
    }

    // wait for threads
    std::unique_lock<std::mutex> ul(mt);
    cond.wait(ul, [&] { return thread_num == finish_num; });

    for (auto &thread : threads) {
        thread.join();
    }

    // ensure the content has been written into the file
    xLog::Flush();

    // Here we simply check the correctness of the number of the logs and logs' format
    std::vector<std::string> lines;
    GetAllLogs(lines, file_io_);

    bool ok = true;
    size_t total_log_num = log_num * thread_num;
    if (lines.size() != total_log_num) {
        ok = false;
    }

    for (size_t i = 0; i < total_log_num && ok; i++) {
        if (!CheckFormat(lines[i].c_str())) {
            ok = false;
        }
    }
    EXPECT_TRUE(ok);
    // remove(log_file);
}

} // namespace basic_test
