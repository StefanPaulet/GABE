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
            PRIVATE ${googletest_SOURCE_DIR}/oogletest)
endif()

set(
    FEATURE_TEST_SOURCES
    DeepBackPropagationTest.cpp
    ConvolutionalDeepBackPropagationTest.cpp
)

add_executable(featuretests
        ${FEATURE_TEST_SOURCES}
        FeatureTest.cpp)

target_include_directories(featuretests PRIVATE ../../src)
target_link_libraries(featuretests lib.gtest)
