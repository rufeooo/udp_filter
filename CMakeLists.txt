cmake_minimum_required(VERSION 2.8.6)

project(udp_filter C)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

include_directories(${PROJECT_SOURCE_DIR})

add_executable(udp_filter udp_filter.c)

target_link_libraries(udp_filter pcre2-8)
