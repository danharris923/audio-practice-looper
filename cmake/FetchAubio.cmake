# Fetch aubio Audio Analysis Library
FetchContent_Declare(
    aubio
    GIT_REPOSITORY https://github.com/aubio/aubio.git
    GIT_TAG        0.4.9
    GIT_SHALLOW    TRUE
)

FetchContent_GetProperties(aubio)
if(NOT aubio_POPULATED)
    FetchContent_Populate(aubio)
    
    # aubio uses waf build system, not CMake
    # We need to create a minimal CMake wrapper
    
    # Set aubio options
    set(AUBIO_BUILD_SHARED OFF)
    set(AUBIO_BUILD_EXAMPLES OFF)
    set(AUBIO_BUILD_TESTS OFF)
    
    # Create aubio CMake target manually
    file(GLOB_RECURSE AUBIO_SOURCES ${aubio_SOURCE_DIR}/src/*.c)
    
    # Filter out unwanted sources
    list(FILTER AUBIO_SOURCES EXCLUDE REGEX ".*aubio_priv\\.h$")
    
    add_library(aubio STATIC ${AUBIO_SOURCES})
    
    target_include_directories(aubio PUBLIC
        ${aubio_SOURCE_DIR}/src
        ${aubio_SOURCE_DIR}
    )
    
    # Generate aubio config.h
    configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/aubio_config.h.in
        ${CMAKE_BINARY_DIR}/aubio_config.h
    )
    
    target_include_directories(aubio PUBLIC ${CMAKE_BINARY_DIR})
    
    # Link math library on Unix
    if(UNIX)
        target_link_libraries(aubio m)
    endif()
    
    # Set compiler definitions
    target_compile_definitions(aubio PRIVATE
        HAVE_CONFIG_H=1
    )
endif()