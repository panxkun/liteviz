cmake_minimum_required(VERSION 3.14)
set(CMAKE_BUILD_TYPE Release)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
project(liteviz)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE BOOL "No dev warnings" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CMakeDependentOption)
include(SuperBuildDepends)
superbuild_option(cpp17)
superbuild_option(modern-cpp)

add_subdirectory(liteviz)

# Option to build examples
option(BUILD_EXAMPLES "Build example applications" OFF)

# Add examples subdirectory if BUILD_EXAMPLES is ON
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()