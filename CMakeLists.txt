cmake_minimum_required(VERSION 3.21)

project(AudioPracticeLooper VERSION 1.0.0)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Compiler flags for warnings and errors
if(MSVC)
    add_compile_options(/W4 /WX)
    add_definitions(-D_WIN32_WINNT=0x0A00)  # Windows 10
else()
    add_compile_options(-Wall -Wextra -Werror -Wpedantic)
endif()

# Build type default to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include FetchContent for dependencies
include(FetchContent)

# Fetch JUCE
include(cmake/FetchJUCE.cmake)

# Fetch Rubber Band Library
include(cmake/FetchRubberBand.cmake)

# Fetch aubio
include(cmake/FetchAubio.cmake)

# Find FFmpeg
include(cmake/FindFFmpeg.cmake)

# Add subdirectories
add_subdirectory(src)

# Enable testing
enable_testing()
add_subdirectory(tests)

# Set output directories
set_target_properties(AudioPracticeLooper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)