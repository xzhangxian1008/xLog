#include <unistd.h>
#include <fcntl.h>

#include "gtest/gtest.h"
#include "xLog.h"

namespace xLog {

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
    buf[target_size] = 0;
    if (read(fd, buf, target_size) == -1) {
        return false;
    }
    
    return strcmp(target, buf) == 0 ? true : false;
}

bool CheckLogFile(const char* log_file, const char* target) {
    if (access(log_file, F_OK) != 0) {
        return false;
    }

    int fd = open(log_file, O_RDONLY);
    if (fd == -1) {
        return false;
    }

    int file_name_size = 0;
    while (target[file_name_size] != 0) {
        file_name_size++;
    }

    // According to the log format, there are some log level data before the file name.
    // 20 is definitely long enough to store these data.
    char buf[20 + file_name_size];
    memset(buf, 0, 20 + file_name_size);

    if (read(fd, buf, 20 + file_name_size) == -1) {
        return -1;
    }

    int file_start_idx = 0;
    while (buf[file_start_idx] != ' ') {
        file_start_idx++;
    }
    file_start_idx++;

    return strcmp(target, buf + file_start_idx) == 0 ? true : false;
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
        return -1;
    }

    int content_start_idx = 0;
    int space = 3;
    while (space > 0) {
        content_start_idx++;
        if (buf[content_start_idx] == ' ') {
            space--;
        }
    }

    return strcmp(target, buf + content_start_idx) == 0 ? true : false;;
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
    int fd = open(log_file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR);
    ASSERT_NE(-1, fd);

    SetLogLevel(LogLevel::DEBUG);
    SetLogFile(log_file);

    const char *log_content = "This is a basic test log";
    X_LOG(DEBUG, log_content);

    // ensure the content has been written into the file
    fsync(fd);

    EXPECT_TRUE(CheckLogLevel(log_file, "[DEBUG]"));
    EXPECT_TRUE(CheckLogFile(log_file, log_file));
    EXPECT_TRUE(CheckLogContent(log_file, log_content));

    close(fd);
    remove(log_file);
}

} // namespace basic_test
