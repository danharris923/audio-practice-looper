set(TARGET_NAME AudioPracticeLooper)

# Source files
set(SOURCES
    main.cpp
    App.cpp
    MainWindow.cpp
    AudioEngine.cpp
    AudioFileSource.cpp
    RubberBandNode.cpp
    EQNode.cpp
    AnalysisWorker.cpp
    WaveformView.cpp
    TransportBar.cpp
    LoopControls.cpp
    SystemAudioCapture.cpp
    ExportEngine.cpp
    ExportDialog.cpp
    DeviceSelector.cpp
    Utils/ParameterSmoother.h  # Header-only
)

# Header files for IDE organization
set(HEADERS
    App.h
    MainWindow.h
    AudioEngine.h
    AudioFileSource.h
    RubberBandNode.h
    EQNode.h
    AnalysisWorker.h
    WaveformView.h
    TransportBar.h
    LoopControls.h
    SystemAudioCapture.h
    ExportEngine.h
    ExportDialog.h
    DeviceSelector.h
    Utils/LockFreeRingBuffer.h
    Utils/ParameterSmoother.h
)

# Create the application target
juce_add_gui_app(${TARGET_NAME}
    PRODUCT_NAME "Audio Practice Looper"
    VERSION "1.0.0"
    COMPANY_NAME "Audio Practice Looper Project"
    BUNDLE_ID "com.audiopracticelooper.app"
    ICON_BIG "${CMAKE_SOURCE_DIR}/resources/icons/icon_512.png"
    ICON_SMALL "${CMAKE_SOURCE_DIR}/resources/icons/icon_64.png"
)

# Add source files
target_sources(${TARGET_NAME} PRIVATE
    ${SOURCES}
    ${HEADERS}
)

# JUCE modules
target_link_libraries(${TARGET_NAME} PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
)

# Third-party libraries
if(TARGET RubberBand)
    target_link_libraries(${TARGET_NAME} PRIVATE RubberBand)
elseif(TARGET rubberband-static)
    target_link_libraries(${TARGET_NAME} PRIVATE rubberband-static)
endif()

if(TARGET aubio)
    target_link_libraries(${TARGET_NAME} PRIVATE aubio)
endif()

if(TARGET FFmpeg::FFmpeg)
    target_link_libraries(${TARGET_NAME} PRIVATE FFmpeg::FFmpeg)
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE
        winmm
        ole32
        user32
        kernel32
        comctl32
        comdlg32
        gdi32
        rpcrt4
        wininet
        version
        ws2_32
        # WASAPI libraries for system audio capture
        mmdevapi
        audioclient
        propsys
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE
        pthread
        dl
        asound  # ALSA
        pulse   # PulseAudio
        pulse-simple  # PulseAudio simple API
    )
endif()

# Preprocessor definitions
target_compile_definitions(${TARGET_NAME} PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${TARGET_NAME},JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${TARGET_NAME},JUCE_VERSION>"
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
)