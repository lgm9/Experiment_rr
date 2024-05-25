cmake_minimum_required(VERSION 3.10)

project(Experiment)

add_subdirectory(Mythread)
add_subdirectory(rocksdb)

set(CMAKE_CXX_STANDARD 17)

add_executable(Experiment main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(Experiment PRIVATE Threads::Threads)
target_link_libraries(Experiment PRIVATE rocksdb)
target_link_libraries(Experiment PUBLIC Mythread)

target_include_directories(Experiment PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/Mythread"
                          )