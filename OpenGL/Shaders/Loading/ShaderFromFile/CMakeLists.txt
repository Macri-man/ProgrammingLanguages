cmake_minimum_required(VERSION 3.16)
project(ShaderFromFile)

set(CMAKE_CXX_STANDARD 20)

# Fetch GLFW
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        latest
)
FetchContent_MakeAvailable(glfw)

# Fetch GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8
    SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(glad)
glad_add_library(glad STATIC API gl:core=4.6)

add_executable(ShaderFromFile main.cpp)
target_link_libraries(ShaderFromFile PRIVATE glfw glad)
