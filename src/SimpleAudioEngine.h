#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>
#include <atomic>

class SimpleAudioEngine : public juce::AudioIODeviceCallback,
                         public juce::ChangeListener
{
public:
    SimpleAudioEngine();
    ~SimpleAudioEngine() override;

    // Initialization
    void initialize();
    void shutdown();

    // File management
    bool loadAudioFile(const juce::File& file);
    void closeAudioFile();
    bool isFileLoaded() const;
    juce::String getCurrentFileName() const;

    // Playback control
    void play();
    void pause();
    void stop();
    bool isPlaying() const;

    // Position control
    void setPosition(double seconds);
    double getPosition() const;
    double getDuration() const;

    // Loop control
    void setLoopEnabled(bool enabled);
    bool getLoopEnabled() const;
    void setLoopStart(double seconds);
    void setLoopEnd(double seconds);
    double getLoopStart() const;
    double getLoopEnd() const;
    
    // Advanced loop controls
    void setLoopAPoint();  // Set A point at current position
    void setLoopBPoint();  // Set B point at current position
    void clearLoop();      // Clear A/B points and continue playing
    void jogLoopStart(double offsetSeconds);  // Fine-tune A point
    void jogLoopEnd(double offsetSeconds);    // Fine-tune B point
    void doubleLoopLength();   // 2x loop length
    void halveLoopLength();    // 0.5x loop length
    void moveLoopRegionBackward();  // Move A->B, B->B+length
    void moveLoopRegionForward();   // Move both points forward by loop length
    
    // Edge bleed and snap settings
    void setEdgeBleedMs(int milliseconds);
    int getEdgeBleedMs() const;
    void setSnapToGrid(bool enabled);
    bool getSnapToGrid() const;

    // Tempo/Pitch (placeholders for now - would need external libs for real implementation)
    void setTempoRatio(float ratio);
    void setPitchSemitones(int semitones);
    float getTempoRatio() const;
    int getPitchSemitones() const;

    // Audio capture/recording
    void startRecording();
    void stopRecording();
    bool isRecording() const;
    void saveRecording(const juce::File& outputFile);
    
    // Input monitoring
    void setInputMonitoring(bool enabled);
    bool getInputMonitoring() const;

    // AudioIODeviceCallback implementation
    void audioDeviceIOCallback(const float** inputChannelData,
                              int numInputChannels,
                              float** outputChannelData,
                              int numOutputChannels,
                              int numSamples) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    // ChangeListener implementation
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    std::unique_ptr<juce::AudioDeviceManager> deviceManager;
    std::unique_ptr<juce::AudioFormatManager> formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::AudioTransportSource> transportSource;

    // State
    std::atomic<bool> isInitialized{false};
    std::atomic<bool> fileLoaded{false};
    juce::String currentFileName;
    
    // Loop parameters
    std::atomic<bool> loopEnabled{false};
    std::atomic<double> loopStartSeconds{0.0};
    std::atomic<double> loopEndSeconds{0.0};
    std::atomic<bool> hasAPoint{false};
    std::atomic<bool> hasBPoint{false};
    
    // Advanced loop settings
    std::atomic<int> edgeBleedMs{5};  // Default 5ms edge bleed
    std::atomic<bool> snapToGrid{false};
    
    // Tempo/Pitch placeholders
    std::atomic<float> tempoRatio{1.0f};
    std::atomic<int> pitchSemitones{0};
    
    // Sample rate for calculations
    double sampleRate = 44100.0;
    
    // Recording/capture state
    std::atomic<bool> isRecordingEnabled{false};
    std::atomic<bool> inputMonitoringEnabled{false};
    std::unique_ptr<juce::AudioSampleBuffer> recordingBuffer;
    std::atomic<int> recordingPosition{0};
    juce::CriticalSection recordingLock;
    
    void setupAudioSources();
    void checkLoopPosition();
    void processInputAudio(const float** inputChannelData, int numInputChannels, int numSamples);
};