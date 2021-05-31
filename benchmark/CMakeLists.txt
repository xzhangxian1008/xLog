cmake_minimum_required(VERSION 3.10)
project(xLog_bench CXX)

find_package(Threads)

set(CMAKE_CXX_STANDARD 17)              # Compile as C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Require C++17 support

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra -Werror -march=native -Wno-reorder")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -O3")

# Includes.
set(XLOG_SRC_INCLUDE_DIR ../src/include)
include_directories(${XLOG_SRC_INCLUDE_DIR})

file(GLOB_RECURSE xLog_src
        ../src/*.cpp ../src/include/*.h)

add_library(xLog_src_lib STATIC ${xLog_src})

add_executable(benchmark benchmark.cpp)

target_link_libraries(benchmark xLog_src_lib ${CMAKE_THREAD_LIBS_INIT})