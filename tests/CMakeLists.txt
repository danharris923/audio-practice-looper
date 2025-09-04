# Test configuration
set(TEST_TARGET AudioPracticeLooperTests)

# Test sources
set(TEST_SOURCES
    test_main.cpp
    AudioEngineTest.cpp
    ParameterSmootherTest.cpp
    LockFreeRingBufferTest.cpp
    ExportEngineTest.cpp
)

# Create test executable
add_executable(${TEST_TARGET} ${TEST_SOURCES})

# Include directories
target_include_directories(${TEST_TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Utils
)

# Link libraries
target_link_libraries(${TEST_TARGET} PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
)

# Third-party test dependencies
if(TARGET aubio)
    target_link_libraries(${TEST_TARGET} PRIVATE aubio)
endif()

if(TARGET RubberBand OR TARGET rubberband-static)
    if(TARGET RubberBand)
        target_link_libraries(${TEST_TARGET} PRIVATE RubberBand)
    else()
        target_link_libraries(${TEST_TARGET} PRIVATE rubberband-static)
    endif()
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${TEST_TARGET} PRIVATE
        winmm ole32 user32 kernel32 comctl32 comdlg32 gdi32 rpcrt4
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${TEST_TARGET} PRIVATE
        pthread dl asound pulse
    )
endif()

# Test discovery
include(CTest)
add_test(NAME AudioPracticeLooperTests COMMAND ${TEST_TARGET})

# Set test properties
set_target_properties(${TEST_TARGET} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)