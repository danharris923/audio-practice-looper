# Fetch Rubber Band Library
FetchContent_Declare(
    RubberBand
    GIT_REPOSITORY https://github.com/breakfastquay/rubberband.git
    GIT_TAG        v4.0.0
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(RubberBand)

# Set Rubber Band options
if(WIN32)
    set(RUBBERBAND_FFTW OFF)
    set(RUBBERBAND_VAMP OFF)
    set(RUBBERBAND_LADSPA OFF)
else()
    # Try to find FFTW3 for better performance on Linux
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FFTW3 fftw3)
        if(FFTW3_FOUND)
            set(RUBBERBAND_FFTW ON)
        else()
            set(RUBBERBAND_FFTW OFF)
        endif()
    else()
        set(RUBBERBAND_FFTW OFF)
    endif()
    
    set(RUBBERBAND_VAMP OFF)
    set(RUBBERBAND_LADSPA OFF)
endif()

# Build static library
set(RUBBERBAND_BUILD_STATIC ON)