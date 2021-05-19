#pragma once

#include <iostream>
#include <string>

namespace xLog {

#define PRINT(info) xLog::print__(__func__, __LINE__, info)

template<typename T>
void print__(std::string func, int line, const T &info) {
    std::cout << func << ", line " << line << ": " << info << std::endl;
}

/**
 * @return true: fail, false: not fail
 */
inline bool IsIOFail(std::fstream &io) {
    if (io.bad() || io.eof() || io.fail()) {
        return true;
    }
    return false;
}

} // namespace xLog
