#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <unistd.h>

#include "gtest/gtest.h"
#include "xLog.h"
#include "util.h"

namespace xLog_basic_test {

/**
 * Only check the correctness of the logs' format
 * 
 * Log format example: [DEBUG] /home/xzx/main.cpp: 345: ...content...
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

bool CheckLogLevel(const char *log_file, const char *target) {
    if (access(log_file, F_OK) != 0) {
        return false;
    }

    int fd = open(log_file, O_RDONLY);
    if (fd == -1) {
        return false;
    }

    ssize_t target_size = 0;
    while (target[target_size] != 0) {
        target_size++;
    }
    
    char buf[target_size+1];
    memset(buf, 0, target_size+1);
    
    lseek(fd, 0, SEEK_SET);
    int read_num = read(fd, buf, target_size);
    if (read_num != target_size) {
        PRINT(read_num);
        return false;
    }


    
    return strcmp(target, buf) == 0 ? true : false;
}

bool CheckLogContent(const char *log_file, const char *target) {
    if (access(log_file, F_OK) != 0) {
        return false;
    }

    int fd = open(log_file, O_RDONLY);
    if (fd == -1) {
        return false;
    }

    int content_size = 0;
    while (target[content_size] != 0) {
        content_size++;
    }

    // According to the log format, there are some log level data before the file name.
    // 100 is definitely long enough to store these data.
    char buf[100 + content_size];
    memset(buf, 0, 100 + content_size);

    if (read(fd, buf, 100 + content_size) == -1) {
        return false;
    }

    int content_start_idx = 0;
    int space = 3;
    while (space > 0) {
        content_start_idx++;
        if (buf[content_start_idx] == ' ') {
            space--;
        }
    }
    content_start_idx++;

    return strcmp(target, buf + content_start_idx) == 0 ? true : false;
}

/**
 * Data ends with '\n' will be converted to the line.
 * '\n' should be peeled off.
 * 
 * @param buf store data that need to be converted
 * @param lines container which stores the converted lines
 */
void GetLines(char *buf, std::vector<std::string> &lines) {
    // refer to the head of a line
    int start_idx = 0;

    // refer to the next char of the end of a line
    int end_idx = 0;

    while (buf[end_idx] != 0) {
        if (buf[end_idx] == '\n') {
            // convert data to the line
            buf[end_idx] = 0;
            lines.emplace_back(std::string(static_cast<char *>(buf + start_idx)));

            start_idx = end_idx + 1;
            end_idx = start_idx;
        }

        end_idx++;
    }
}

/**
 * Log format example: [DEBUG] /home/xzx/main.cpp: 345: ...content...
 * 
 * Write only one log into file
 */
TEST(BasicLogTest, DISABLED_BasicTest) {
    const char *log_file = "basic_log_test.txt";
    remove(log_file);

    // create file
    std::fstream file_io_(log_file, std::ios::in | std::ios::out | std::ios::trunc);

    xLog::SetLogLevel(xLog::LogLevel::DEBUG);
    xLog::SetLogFile(log_file);

    const char *log_content = "This is a basic test log";
    X_LOG(xLog::DEBUG, log_content);
    
    // ensure the content has been written into the file
    xLog::Flush();

    EXPECT_TRUE(CheckLogLevel(log_file, "[DEBUG]"));

    std::string content(log_content);
    content.append("\n");
    EXPECT_TRUE(CheckLogContent(log_file, content.c_str()));

    // remove(log_file);
}

/**
 * Record a bunch of logs in a single thread
 */
TEST(BasicLogTest, SingleThreadManyLogsTest) {
    const char *log_file = "single_thd_many_logs_test.txt";
    remove(log_file);

    // create file
    std::fstream file_io_(log_file, std::ios::in | std::ios::out | std::ios::trunc);

    xLog::SetLogLevel(xLog::LogLevel::DEBUG);
    xLog::SetLogFile(log_file);

    // create a lot of logs
    int log_num = 1000000;
    std::string log_content("pretend to have many many info...");
    for (int i = 0; i < log_num; i++) {
        X_LOG(xLog::LogLevel::DEBUG, log_content.c_str());
    }

    // ensure the content has been written into the file
    xLog::Flush();

    int read_buf_size = 100000;
    char buf[read_buf_size + 1];
    memset(buf, 0, read_buf_size + 1);
    std::vector<std::string> lines;

    // read all the logs and check their formats
    file_io_.seekp(0);
    file_io_.read(buf, read_buf_size);
    int read_cnt = file_io_.gcount();
    while (read_cnt > 0) {
        GetLines(buf, lines);
        memset(buf + read_cnt, 0, read_buf_size - read_cnt);
        if (buf[read_cnt - 1] != '\n') {
            // We read an incomplete log
            // Back the cursor to the head of the log
            
        }
        read_cnt = file_io_.gcount();
    }

    // bool ok = true;
    // for (auto &line : lines) {
    //     if (!CheckFormat(line.c_str())) {
    //         ok = false;
    //         break;
    //     }
    // }
    // EXPECT_TRUE(ok);
}

/**
 * Record a bunch of logs in multi-threads
 */
TEST(BasicLogTest, DISABLED_MultiThreadManyLogsTest) {

}

} // namespace basic_test
