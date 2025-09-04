#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <memory>
#include <atomic>

#include "AudioFileSource.h"
#include "RubberBandNode.h"
#include "EQNode.h"
#include "AnalysisWorker.h"
#include "Utils/LockFreeRingBuffer.h"
#include "Utils/ParameterSmoother.h"

class AudioEngine : public juce::AudioIODeviceCallback,
                   public juce::AudioProcessorGraph::AudioGraphIOProcessor::Callback
{
public:
    AudioEngine();
    ~AudioEngine();

    void initialize();
    void shutdown();

    // File management
    bool loadAudioFile(const juce::File& file);
    void closeAudioFile();

    // Playback control
    void play();
    void pause();
    void stop();
    bool isPlaying() const;

    // Parameter control
    void setTempoRatio(float ratio);
    void setPitchSemitones(int semitones);
    
    // Loop control
    void setLoopInSeconds(double seconds);
    void setLoopOutSeconds(double seconds);
    void setLoopEnabled(bool enabled);

    // Analysis control
    AnalysisResult getAnalysisResults() const;
    void setAnalysisEnabled(bool enabled);

    // AudioIODeviceCallback implementation
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                        int numInputChannels,
                                        float* const* outputChannelData,
                                        int numOutputChannels,
                                        int numSamples,
                                        const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

private:
    std::unique_ptr<juce::AudioDeviceManager> deviceManager;
    std::unique_ptr<juce::AudioProcessorGraph> processorGraph;
    
    // Node IDs for the processor graph
    juce::AudioProcessorGraph::NodeID audioInputNodeID;
    juce::AudioProcessorGraph::NodeID audioOutputNodeID;
    juce::AudioProcessorGraph::NodeID fileSourceNodeID;
    juce::AudioProcessorGraph::NodeID rubberBandNodeID;
    juce::AudioProcessorGraph::NodeID eqNodeID;

    // Audio components
    std::unique_ptr<AudioFileSource> fileSource;
    std::unique_ptr<RubberBandNode> rubberBandNode;
    std::unique_ptr<EQNode> eqNode;
    std::unique_ptr<AnalysisWorker> analysisWorker;

    // Parameter smoothing
    ParameterSmoother<float> tempoSmoother;
    ParameterSmoother<float> pitchSmoother;
    
    // State management
    std::atomic<float> tempoRatio{1.0f};
    std::atomic<int> pitchSemitones{0};
    std::atomic<bool> isPlaying_{false};
    std::atomic<bool> loopEnabled{false};
    std::atomic<double> loopInSeconds{0.0};
    std::atomic<double> loopOutSeconds{0.0};

    // Audio processing
    double sampleRate = 44100.0;
    int blockSize = 512;
    
    void setupAudioGraph();
    void updateParameters();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};