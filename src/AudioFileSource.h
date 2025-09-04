#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <atomic>
#include <memory>

// Forward declarations for FFmpeg types
extern "C" 
{
    struct AVFormatContext;
    struct AVCodecContext;
    struct AVFrame;
    struct AVPacket;
    struct SwrContext;
}

class AudioFileSource : public juce::AudioProcessor
{
public:
    AudioFileSource();
    ~AudioFileSource() override;

    // File operations
    bool loadFile(const juce::File& file);
    void closeFile();
    bool isFileLoaded() const;

    // Playback control
    void setPlaybackPosition(double positionInSeconds);
    double getCurrentPosition() const;
    double getTotalLength() const;
    
    // Loop control
    void setLoopPoints(double startSeconds, double endSeconds);
    void setLoopEnabled(bool enabled);

    // AudioProcessor overrides
    const juce::String getName() const override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

private:
    // FFmpeg context
    AVFormatContext* formatContext = nullptr;
    AVCodecContext* codecContext = nullptr;
    SwrContext* swrContext = nullptr;
    AVFrame* frame = nullptr;
    AVPacket* packet = nullptr;
    
    int audioStreamIndex = -1;
    
    // Audio properties
    double sampleRate = 44100.0;
    int numChannels = 2;
    double totalDurationSeconds = 0.0;
    
    // Playback state
    std::atomic<double> currentPositionSeconds{0.0};
    std::atomic<bool> fileLoaded{false};
    
    // Loop state
    std::atomic<bool> loopEnabled{false};
    std::atomic<double> loopStartSeconds{0.0};
    std::atomic<double> loopEndSeconds{0.0};
    
    // Internal methods
    bool initializeFFmpeg();
    void cleanupFFmpeg();
    bool openCodec();
    bool setupResampler();
    int readNextFrame(float* outputBuffer, int numSamplesToRead);
    void seekToPosition(double seconds);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFileSource)
};