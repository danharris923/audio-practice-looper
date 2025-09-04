#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <atomic>

#include "Utils/ParameterSmoother.h"

// Forward declaration for Rubber Band
namespace RubberBand
{
    class RubberBandStretcher;
}

class RubberBandNode : public juce::AudioProcessor
{
public:
    RubberBandNode();
    ~RubberBandNode() override;

    // AudioProcessor overrides
    const juce::String getName() const override { return "RubberBandNode"; }
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    // Control methods
    void setTimeRatio(float ratio);
    void setPitchScale(float scale);

private:
    // Rubber Band stretcher
    std::unique_ptr<RubberBand::RubberBandStretcher> stretcher;
    
    // Parameter smoothing
    ParameterSmoother<float> timeRatioSmoother;
    ParameterSmoother<float> pitchScaleSmoother;
    
    // Atomic parameters for thread-safe access
    std::atomic<float> timeRatio{1.0f};
    std::atomic<float> pitchScale{1.0f};
    
    // Audio processing state
    double sampleRate = 44100.0;
    int blockSize = 512;
    int numChannels = 2;
    
    // Processing buffers
    std::vector<float> tempBuffer;
    std::vector<const float*> inputPointers;
    std::vector<float*> outputPointers;
    
    void initializeStretcher();
    void updateParameters();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RubberBandNode)
};