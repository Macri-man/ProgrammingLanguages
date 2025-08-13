cmake_minimum_required(VERSION 3.14)
project(OpenALWaveformDisplay)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- Fetch OpenAL Soft ---
FetchContent_Declare(
    openal
    GIT_REPOSITORY https://github.com/kcat/openal-soft.git
    GIT_TAG        master  # You can pin to a specific version or commit
)
FetchContent_MakeAvailable(openal)

# --- Add your sources ---
add_executable(OpenALWaveformDisplay main.cpp)

# --- Link to OpenAL Soft ---
target_link_libraries(OpenALWaveformDisplay PRIVATE OpenAL)

# --- Include OpenAL headers ---
target_include_directories(OpenALWaveformDisplay PRIVATE
    ${openal_SOURCE_DIR}/include
)

# --- Optional: Enable EFX in OpenAL Soft ---
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_NO_CONFIG_UTIL OFF CACHE BOOL "" FORCE)
set(ALSOFT_REQUIRE_EFX ON CACHE BOOL "" FORCE) # This ensures EFX extension is built in

message(STATUS "OpenAL Soft will be built with EFX support")
