cmake_minimum_required(VERSION 3.15)
project(GABE)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(COMPILER_SPECIFIC_COVERAGE_FLAGS "-fcoverage-mapping -fprofile-instr-generate -O0 -g")
else()
  set(COMPILER_SPECIFIC_COVERAGE_FLAGS "")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_SPECIFIC_COVERAGE_FLAGS}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
set(CDS_INCLUDE_DIRECTORIES CDS/include)

add_executable(
    server
    src/target/server.cpp
    src/server/Server.cpp
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
    ${CDS_INCLUDE_DIRECTORIES}
)

target_include_directories(
    main
    PUBLIC
    src
    ${CDS_INCLUDE_DIRECTORIES}
)

target_link_libraries(main PUBLIC X11 Xtst jpeg)

include(FetchContent)
enable_testing()
add_subdirectory(test/unittest)
add_subdirectory(test/featuretest)
