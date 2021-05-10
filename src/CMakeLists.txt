# reference to the CMU Database Group

file(GLOB_RECURSE xLog_sources ${PROJECT_SOURCE_DIR}/src/*/*.cpp ${PROJECT_SOURCE_DIR}/src/*/*/*.cpp ${PROJECT_SOURCE_DIR}/src/*/*/*/*.cpp)
add_library(xLog_shared SHARED ${xLog_sources})