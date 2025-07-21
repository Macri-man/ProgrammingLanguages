cmake_minimum_required(VERSION 3.15)
project(OpenGLRayTracing)

set(CMAKE_CXX_STANDARD 20)


# Required for FetchContent
include(FetchContent)

# === GLAD ===
FetchContent_Declare(
	glad
	GIT_REPOSITORY https://github.com/Dav1dde/glad.git
	GIT_TAG v2.0.8
  GIT_SHALLOW TRUE
  SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(glad)

glad_add_library(glad STATIC API gl:core=4.6)

# === GLFW ===
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

# === GLM ===
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# === OpenGL ===
find_package(OpenGL REQUIRED)

# === Executable ===
add_executable(OpenGLRayTracing main.cpp)

# === Include and Link ===
target_link_libraries(OpenGLRayTracing PRIVATE glad glfw glm)
