cmake_minimum_required(VERSION 3.15)
project(GABE)

set(CMAKE_CXX_STANDARD 20)

add_executable(
    server
    src/target/server.cpp
    src/server/Server.cpp
    src/server/thread/Thread.cpp
)

target_include_directories(
    server
    PUBLIC
    src/server
)