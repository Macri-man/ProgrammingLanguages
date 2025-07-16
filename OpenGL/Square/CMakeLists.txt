cmake_minimum_required(VERSION 3.15)
project(OpenGLSquare)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)

add_subdirectory(../glfw ${CMAKE_BINARY_DIR}/glfw)

# Create executable
add_executable(OpenGLSquare main.cpp ../glad/src/glad.c)

# Include directories
target_include_directories(OpenGLSquare PRIVATE 
    glad/include 
    glfw/include
)

# Link everything
target_link_libraries(OpenGLSquare PRIVATE 
    glfw 
    ${OPENGL_LIBRARIES}
)