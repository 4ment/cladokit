cmake_minimum_required(VERSION 3.15)
project(cladokit VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_TESTING "Enable building of tests" OFF)

file(GLOB_RECURSE CLADOKIT_SOURCES CONFIGURE_DEPENDS src/cladokit/*.cpp src/cladokit/*.hpp)

add_library(cladokit ${CLADOKIT_SOURCES})
target_include_directories(cladokit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(BUILD_TESTING)
    enable_testing()

    # GoogleTest setup
    find_package(GTest REQUIRED)
    include(GoogleTest)

    file(GLOB TEST_SOURCES tests/*.cpp)
    add_executable(run_tests ${TEST_SOURCES})

    target_include_directories(run_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(run_tests PRIVATE cladokit GTest::gtest_main)

    gtest_discover_tests(run_tests)
endif()

# clang-format target
find_program(CLANG_FORMAT_BIN clang-format)

if(CLANG_FORMAT_BIN)
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT_BIN} -i ${CLADOKIT_SOURCES} ${TEST_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-format on source files"
    )
else()
    message(WARNING "clang-format not found")
endif()