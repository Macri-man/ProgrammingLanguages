cmake_minimum_required(VERSION 3.15)
project(VulkanSquare)

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
set(GLFW_INCLUDE_NONE ON CACHE BOOL "")
FetchContent_MakeAvailable(glfw)

# === Vulkan ===
FetchContent_Declare(
  vulkan_headers
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG v1.4.325
)
FetchContent_MakeAvailable(vulkan_headers)

FetchContent_Declare(
  vulkan_loader
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
  GIT_TAG v1.4.325
)
FetchContent_GetProperties(vulkan_loader)
if(NOT vulkan_loader_POPULATED)
    FetchContent_Populate(vulkan_loader)

    if(UNIX)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E chmod +x
                    ${vulkan_loader_SOURCE_DIR}/loader/asm_offset
        )
    endif()
    
    add_subdirectory(${vulkan_loader_SOURCE_DIR} ${vulkan_loader_BINARY_DIR})
endif()

# === SPVShaders ===
file(GLOB SHADERS "shaders/*.vert" "shaders/*.frag")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders.spv")

foreach(SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/shaders.spv/${FILE_NAME}.spv"
        COMMAND glslangValidator -V ${SHADER} -o "${CMAKE_BINARY_DIR}/shaders.spv/${FILE_NAME}.spv"
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILE_NAME}"
        VERBATIM
    )
    list(APPEND SPV_SHADERS "${CMAKE_BINARY_DIR}/shaders.spv/${FILE_NAME}.spv")
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPV_SHADERS})

# === Executable ===
add_executable(VulkanSquare main.cpp ${SPV_SHADERS})
add_dependencies(VulkanSquare Shaders)

# === Include and Link ===
target_include_directories(VulkanSquare PRIVATE
    ${vulkan_headers_SOURCE_DIR}/include
    ${vulkan_loader_SOURCE_DIR}/loader
    ${glfw_SOURCE_DIR}/include
)

target_link_libraries(VulkanSquare PRIVATE glfw vulkan_loader dl pthread)