# Find FFmpeg libraries
# Sets: FFMPEG_FOUND, FFMPEG_LIBRARIES, FFMPEG_INCLUDE_DIRS

find_package(PkgConfig QUIET)

# Try pkg-config first
if(PkgConfig_FOUND)
    pkg_check_modules(PC_FFMPEG QUIET 
        libavcodec>=58.0.0 
        libavformat>=58.0.0 
        libavutil>=56.0.0
        libswresample>=3.0.0)
    
    if(PC_FFMPEG_FOUND)
        set(FFMPEG_FOUND TRUE)
        set(FFMPEG_LIBRARIES ${PC_FFMPEG_LIBRARIES})
        set(FFMPEG_INCLUDE_DIRS ${PC_FFMPEG_INCLUDE_DIRS})
        set(FFMPEG_LINK_DIRECTORIES ${PC_FFMPEG_LIBRARY_DIRS})
        
        message(STATUS "Found FFmpeg via pkg-config")
        return()
    endif()
endif()

# Manual search for FFmpeg components
find_path(AVCODEC_INCLUDE_DIR
    NAMES libavcodec/avcodec.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        ${CMAKE_PREFIX_PATH}/include
)

find_path(AVFORMAT_INCLUDE_DIR
    NAMES libavformat/avformat.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        ${CMAKE_PREFIX_PATH}/include
)

find_library(AVCODEC_LIBRARY
    NAMES avcodec
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        ${CMAKE_PREFIX_PATH}/lib
)

find_library(AVFORMAT_LIBRARY
    NAMES avformat
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        ${CMAKE_PREFIX_PATH}/lib
)

find_library(AVUTIL_LIBRARY
    NAMES avutil
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        ${CMAKE_PREFIX_PATH}/lib
)

find_library(SWRESAMPLE_LIBRARY
    NAMES swresample
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        ${CMAKE_PREFIX_PATH}/lib
)

# Check if all required components are found
if(AVCODEC_INCLUDE_DIR AND AVFORMAT_INCLUDE_DIR AND 
   AVCODEC_LIBRARY AND AVFORMAT_LIBRARY AND AVUTIL_LIBRARY AND SWRESAMPLE_LIBRARY)
    
    set(FFMPEG_FOUND TRUE)
    set(FFMPEG_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR})
    set(FFMPEG_LIBRARIES 
        ${AVCODEC_LIBRARY} 
        ${AVFORMAT_LIBRARY} 
        ${AVUTIL_LIBRARY}
        ${SWRESAMPLE_LIBRARY})
    
    message(STATUS "Found FFmpeg libraries manually")
else()
    set(FFMPEG_FOUND FALSE)
    message(WARNING "FFmpeg not found. Some audio format support may be limited.")
    
    # Create dummy targets for optional FFmpeg functionality
    add_library(ffmpeg_dummy INTERFACE)
    set(FFMPEG_LIBRARIES ffmpeg_dummy)
    set(FFMPEG_INCLUDE_DIRS "")
endif()

# Create imported targets for cleaner usage
if(FFMPEG_FOUND AND NOT TARGET FFmpeg::FFmpeg)
    add_library(FFmpeg::FFmpeg INTERFACE IMPORTED)
    set_target_properties(FFmpeg::FFmpeg PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${FFMPEG_LIBRARIES}"
    )
endif()