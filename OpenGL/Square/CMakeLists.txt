cmake_minimum_required(VERSION 3.15)
project(OpenGLTriangle)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

# GLFW
add_subdirectory(../glfw ${CMAKE_BINARY_DIR}/glfw)

# Executable
add_executable(OpenGLTriangle main.cpp ../glad/src/glad.c)

target_link_libraries(OpenGLTriangle PRIVATE glfw ${OpenGL_LIBRARIES})
target_include_directories(OpenGLTriangle PRIVATE ../glad/include ../glfw/include)
