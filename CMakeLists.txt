cmake_minimum_required(VERSION 3.15)
project(cladokit VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_TESTING "Enable building of tests" OFF)

file(GLOB_RECURSE CLADOKIT_SOURCES CONFIGURE_DEPENDS src/cladokit/*.cpp src/cladokit/*.hpp)

add_library(cladokit ${CLADOKIT_SOURCES})

target_include_directories(cladokit PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

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

install(TARGETS cladokit
        EXPORT cladokit-targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

# Install header files
install(DIRECTORY src/cladokit/ DESTINATION include/cladokit FILES_MATCHING PATTERN "*.hpp")

# Install the CMake export target file
install(EXPORT cladokit-targets
        FILE cladokitTargets.cmake
        NAMESPACE cladokit::
        DESTINATION lib/cmake/cladokit
)

# Install the config file so `find_package(cladokit)` works
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cladokitConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cladokitConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cladokitConfig.cmake
    INSTALL_DESTINATION lib/cmake/cladokit
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cladokitConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cladokitConfigVersion.cmake
    DESTINATION lib/cmake/cladokit
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Building cladokit with shared libraries: ${BUILD_SHARED_LIBS}")
message(STATUS "Install with: cmake --install ${CMAKE_BINARY_DIR}")
message(STATUS "Uninstall with: cmake --build ${CMAKE_BINARY_DIR} --target uninstall ")
