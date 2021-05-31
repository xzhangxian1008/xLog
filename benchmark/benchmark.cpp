#include <iostream>
#include <chrono>
#include <string>
#include <stdio.h>

#include "xLog.h"
#include "util.h"

void benchmark(int howmany) {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;

    auto start = high_resolution_clock::now();
    for (auto i = 0; i < howmany; ++i) {
        X_LOG(xLog::DEBUG, "%s %s %s%d", "This", "is", "log", 233);
    }
    xLog::Flush();
    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();

    printf("Log number:%d Elapsed: %.3lfsec %d/sec\n", howmany, delta_d, static_cast<int>(howmany/delta_d));
}

int main() {
    const char *log_file = "single_thd_bench.txt";
    remove(log_file);

    // create file
    std::fstream file_io_(log_file, std::ios::in | std::ios::out | std::ios::trunc);

    xLog::SetLogLevel(xLog::LogLevel::DEBUG);
    xLog::SetLogFile(log_file);
    
    benchmark(1000000);
    
    return 0;
}