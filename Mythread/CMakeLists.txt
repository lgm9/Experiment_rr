set(CMAKE_CXX_STANDARD 17)

add_library(Mythread worker.cpp scheduler.cpp payload.cpp)

target_link_libraries(Mythread PRIVATE rocksdb)