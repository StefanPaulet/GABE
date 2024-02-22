FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 9b12f74)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_library(lib.gtest
      ${googletest_SOURCE_DIR}/googletest/src/gtest-all.cc)
  target_include_directories(lib.gtest
      PUBLIC ${googletest_SOURCE_DIR}/googletest/include/
      PRIVATE ${googletest_SOURCE_DIR}/googletest)
endif()

set(
    UNIT_TEST_SOURCES
    LinearArrayTest.cpp
    LinearMatrixTest.cpp
    NeuralNetTest.cpp
    FunctionTest.cpp
    LayerTest.cpp
    PredicatesTest.cpp
)

add_executable(unittests
    ${UNIT_TEST_SOURCES}
    UnitTest.cpp)

target_include_directories(unittests PRIVATE ../../src)
target_link_libraries(unittests lib.gtest)
