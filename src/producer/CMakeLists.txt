cmake_minimum_required(VERSION 4.0.0)

project(producer VERSION 0.0.1 LANGUAGES C CXX)

option(PRODUCER_WITH_SANITIZERS "[producer] Enable sanitizers" OFF)

if(PRODUCER_WITH_SANITIZERS)
    message(STATUS "[producer] Sanitizers enabled")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -march=native" CACHE STRING "" FORCE)
endif()

add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)

target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE common)
