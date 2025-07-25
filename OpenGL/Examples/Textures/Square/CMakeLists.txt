cmake_minimum_required(VERSION 3.14)
project(OpenGLTextureExample)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)


# GLFW using FetchContent
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# STB using FetchContent
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# GLAD using FetchContent
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8
    SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(glad)
glad_add_library(glad STATIC API gl:core=4.6)

add_executable(OpenGLTextureExample main.cpp)

target_link_libraries(OpenGLTextureExample PRIVATE glad glfw) 

target_include_directories(OpenGLTextureExample PRIVATE ${stb_SOURCE_DIR} )
