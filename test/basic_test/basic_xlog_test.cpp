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

bool CheckLogLevel(const char* log_file, const char* target) {
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


bool CheckLogContent(const char* log_file, const char* target) {
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

} // namespace basic_test
