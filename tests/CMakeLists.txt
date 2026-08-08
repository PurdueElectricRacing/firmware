cmake_minimum_required(VERSION 3.21)
project(firmware_host_tests LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

set(PER_TEST_LAYER "all" CACHE STRING "Host test layer to build: all, unit, or integration")
set_property(CACHE PER_TEST_LAYER PROPERTY STRINGS all unit integration)
if(NOT PER_TEST_LAYER MATCHES "^(all|unit|integration)$")
    message(FATAL_ERROR "PER_TEST_LAYER must be all, unit, or integration")
endif()

include(FetchContent)
find_package(GTest CONFIG QUIET)
if(NOT TARGET GTest::gtest_main)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
        URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)
if(PER_TEST_LAYER STREQUAL "all" OR PER_TEST_LAYER STREQUAL "unit")
    add_subdirectory(unit)
endif()
if(PER_TEST_LAYER STREQUAL "all" OR PER_TEST_LAYER STREQUAL "integration")
    add_subdirectory(integration)
endif()
