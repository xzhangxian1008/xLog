#include <iostream>
#include <chrono>
#include <string>
#include <stdio.h>
#include <vector>
#include <thread>

#include "xLog.h"
#include "util.h"

void benchmark(int how_many) {
    for (auto i = 0; i < how_many; ++i) {
        // TODO here we should replace this clause with a macro that defined by the user.
        // we could refer to the method of the Nanolog
        X_LOG(xLog::DEBUG, "%s %s %s%d", "This", "is", "log", 233);
    }
}

void single_thd_benchmark(int how_many) {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;

    auto start = high_resolution_clock::now();
    benchmark(how_many);
    xLog::Flush();
    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();

    printf("Thread number:%d Log number:%d Elapsed: %.3fsec %d/sec\n",
        1, how_many, delta_d, static_cast<int>(how_many/delta_d));
}

void multi_thd_benchmark(int how_many, int thd_num) {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;

    auto start = high_resolution_clock::now();
    std::vector<std::thread> threads;
    
    for (int i = 0; i < thd_num; i++) {
        std::thread thd(benchmark, how_many);
        threads.push_back(std::move(thd));
    }

    // I think that we don't need complex synchronization here
    // If this loop will consume too much time? I don't know so far
    for (auto &thd : threads) {
        thd.join();
    }

    xLog::Flush();
    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();

    printf("Thread number:%d Log number:%d Elapsed: %.3fsec %d/sec\n",
        thd_num, how_many*thd_num, delta_d, static_cast<int>(how_many*thd_num/delta_d));
}

int main() {
    {
        // single thread benchmark
        const char *log_file = "single_thd_bench.txt";
        remove(log_file);

        // create file
        std::fstream file_io_(log_file, std::ios::in | std::ios::out | std::ios::trunc);

        xLog::SetLogLevel(xLog::LogLevel::DEBUG);
        xLog::SetLogFile(log_file);
        
        single_thd_benchmark(1000000);
        remove(log_file);
    }

    {
        // multi-threads benchmark
        const char *log_file = "multi_thd_bench.txt";
        remove(log_file);

        // create file
        std::fstream file_io_(log_file, std::ios::in | std::ios::out | std::ios::trunc);

        xLog::SetLogLevel(xLog::LogLevel::DEBUG);
        xLog::SetLogFile(log_file);

        multi_thd_benchmark(500000, 8);
        remove(log_file);
    }
    
    return 0;
}