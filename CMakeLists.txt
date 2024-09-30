cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

project(wecs)

add_library(wecs INTERFACE)
target_include_directories(wecs INTERFACE "./")

option(WECS_BUILD_TEST "build test" OFF)
if(PROJECT_IS_TOP_LEVEL OR WECS_BUILD_TEST)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()