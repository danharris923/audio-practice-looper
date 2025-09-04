# Fetch JUCE Framework
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(JUCE)

# Set JUCE options
set(JUCE_BUILD_TOOLS OFF)
set(JUCE_BUILD_EXAMPLES OFF)
set(JUCE_BUILD_EXTRAS OFF)

# Add JUCE modules path
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${juce_SOURCE_DIR})