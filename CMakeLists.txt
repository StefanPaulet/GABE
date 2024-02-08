cmake_minimum_required(VERSION 3.15)
project(GABE)

set(CMAKE_CXX_STANDARD 20)

add_executable(
    server
    src/target/server.cpp
    src/server/Server.cpp
    src/server/thread/Thread.cpp
    src/server/httpMessage/HttpMessage.cpp
    src/server/socket/Socket.cpp
)

add_executable(
    client
    src/target/client.cpp
)

add_executable(
    main
    src/target/main.cpp
)

target_include_directories(
    server
    PUBLIC
    src/server
)

target_include_directories(
    main
    PUBLIC
    src
)

include(FetchContent)
enable_testing()
add_subdirectory(test/unittest)
