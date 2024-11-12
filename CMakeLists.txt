cmake_minimum_required(VERSION 3.11)
project(liteviz LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Add compile options
add_compile_options(-fPIC)

# Find required packages
find_package(GLEW REQUIRED)
message(STATUS "Found GLEW in ${GLEW_INCLUDE_DIRS}")
find_package(glfw3 REQUIRED)
message(STATUS "Found GLFW3 in ${glfw3_INCLUDE_DIRS}")
find_package(OpenCV REQUIRED)
message(STATUS "Found OpenCV in ${OpenCV_INCLUDE_DIRS}")
find_package(OpenGL REQUIRED)
message(STATUS "Found OpenGL in ${OPENGL_INCLUDE_DIR}")

# Include necessary directories
include_directories(
  ${CMAKE_SOURCE_DIR}/imgui
  ${CMAKE_SOURCE_DIR}/imgui/backends
  ${GLEW_INCLUDE_DIRS}
  ${glfw3_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
)

# Find source files
file(GLOB IMGUI_SOURCE
  ${CMAKE_SOURCE_DIR}/imgui/*.h
  ${CMAKE_SOURCE_DIR}/imgui/*.cpp
  ${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.*
  ${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.*
)

file(GLOB LITEVIZ_SOURCE
  ${CMAKE_SOURCE_DIR}/core/*.cpp
  ${CMAKE_SOURCE_DIR}/core/*.h
)

add_library(liteviz STATIC
  ${IMGUI_SOURCE}
  ${LITEVIZ_SOURCE}
)

target_link_libraries(liteviz
  PRIVATE
    glfw
    GLEW::GLEW
    ${OpenCV_LIBS}
    ${OPENGL_LIBRARIES}
)

target_include_directories(liteviz PUBLIC ${CMAKE_SOURCE_DIR}/core)

add_executable(viewer ${CMAKE_SOURCE_DIR}/app/main.cpp)
target_link_libraries(viewer
  PRIVATE
    liteviz
    glfw
    GLEW::GLEW
    ${OpenCV_LIBS}
    ${OPENGL_LIBRARIES}
)