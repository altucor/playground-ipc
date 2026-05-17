cmake_minimum_required(VERSION 4.0.0)

project(consumer VERSION 0.0.1 LANGUAGES C CXX)

option(CONSUMER_WITH_SANITIZERS "[consumer] Enable sanitizers" OFF)

if(CONSUMER_WITH_SANITIZERS)
    message(STATUS "[consumer] Sanitizers enabled")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)

target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE common)
