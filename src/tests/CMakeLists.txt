cmake_minimum_required(VERSION 4.0.0)

project(tests LANGUAGES C CXX)

include(GoogleTest)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES "./src/**.cpp")

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)

target_link_libraries(${PROJECT_NAME} PRIVATE common)
target_link_libraries(${PROJECT_NAME}
    PRIVATE GTest::gtest
    PRIVATE GTest::gtest_main
    PRIVATE ${TOP_LEVEL_PROJECT})

gtest_discover_tests(${PROJECT_NAME})
