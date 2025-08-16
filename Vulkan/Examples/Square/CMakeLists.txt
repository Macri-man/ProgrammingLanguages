cmake_minimum_required(VERSION 3.15)
project(VulkanTriangle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Required for FetchContent
include(FetchContent)

# === GLFW ===
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

# === Vulkan ===
FetchContent_Declare(
  vulkan_headers
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG v1.4.323
)
FetchContent_MakeAvailable(vulkan_headers)

FetchContent_Declare(
  vulkan_loader
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
  GIT_TAG v1.4.323
)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(vulkan_loader)

# === SPVShaders ===
file(GLOB SHADERS "shaders/*.vert" "shaders/*.frag")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders.spv")

foreach(SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/shaders.spv/${FILE_NAME}.spv"
        COMMAND glslangValidator -V ${SHADER} -o "${CMAKE_BINARY_DIR}/shaders.spv/${FILE_NAME}.spv"
        DEPENDS ${SHADER}
    )
    list(APPEND SPV_SHADERS "${CMAKE_BINARY_DIR}/shaders.spv/${FILE_NAME}.spv")
endforeach()

# === Executable ===
add_executable(VulkanTriangle main.cpp ${SPV_SHADERS})

# === Include and Link ===
target_link_libraries(VulkanTriangle PRIVATE glfw)