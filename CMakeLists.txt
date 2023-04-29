cmake_minimum_required(VERSION 3.25)
project(yui)

set(CMAKE_CXX_STANDARD 20)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

add_subdirectory(yui)
add_executable(test yui/main.cpp)
target_include_directories(test PUBLIC yui)
target_link_libraries(test yui fmt::fmt spdlog::spdlog)
