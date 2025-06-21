cmake_minimum_required(VERSION 3.15)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)

# GLFW
add_subdirectory(glfw)
include_directories(glfw/include)

# Vulkan
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIR})

# Executable
add_executable(GameEngine main.cpp)
target_link_libraries(GameEngine glfw ${Vulkan_LIBRARY})