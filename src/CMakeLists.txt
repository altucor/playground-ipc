cmake_minimum_required(VERSION 4.0.0)

project(god-project LANGUAGES C CXX)

add_subdirectory(common)
add_subdirectory(producer)
add_subdirectory(consumer)

option(ENABLE_TESTS "Enable building tests" OFF)

if(ENABLE_TESTS)
    message(STATUS "Building with unit-tests")
    enable_testing()
    include(FetchContent)

    # Declare the content to fetch (Google Test)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.x
    )

    FetchContent_MakeAvailable(googletest)
    add_subdirectory(tests)
endif()
