cmake_minimum_required(VERSION 3.24)
project(parSort)

set(CMAKE_CXX_STANDARD 20)

find_package(TBB REQUIRED)

include_directories(${TBB_INCLUDE_DIRS})
link_directories(${TBB_LIBRARY_DIRS})

add_executable(parSort main.cpp)
target_link_libraries(parSort TBB::tbb)