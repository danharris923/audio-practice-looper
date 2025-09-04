#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

#include "Utils/ParameterSmoother.h"

class EQNode : public juce::AudioProcessor
{
public:
    // EQ band parameters
    struct BandParameters
    {
        std::atomic<float> frequency{1000.0f};
        std::atomic<float> gain{0.0f};        // dB
        std::atomic<float> q{0.707f};         // Q factor
        std::atomic<bool> enabled{true};
    };

    EQNode();
    ~EQNode() override;

    // AudioProcessor overrides
    const juce::String getName() const override { return "EQNode"; }
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

    // EQ Control methods
    void setLowShelfGain(float gainDb);
    void setLowShelfFrequency(float frequency);
    
    void setPeakGain(float gainDb);
    void setPeakFrequency(float frequency);
    void setPeakQ(float q);
    
    void setHighShelfGain(float gainDb);
    void setHighShelfFrequency(float frequency);
    
    void setBypassEnabled(bool bypassed);

private:
    using FilterType = juce::dsp::IIR::Filter<float>;
    using ProcessorChain = juce::dsp::ProcessorChain<FilterType, FilterType, FilterType>;

    // DSP processor chain: Low Shelf -> Peak -> High Shelf
    ProcessorChain processorChain;
    
    // Band parameters
    BandParameters lowShelf;    // Low shelf filter
    BandParameters peak;        // Peak filter  
    BandParameters highShelf;   // High shelf filter
    
    // Parameter smoothers
    ParameterSmoother<float> lowShelfGainSmoother;
    ParameterSmoother<float> lowShelfFreqSmoother;
    ParameterSmoother<float> peakGainSmoother;
    ParameterSmoother<float> peakFreqSmoother;
    ParameterSmoother<float> peakQSmoother;
    ParameterSmoother<float> highShelfGainSmoother;
    ParameterSmoother<float> highShelfFreqSmoother;

    // State
    std::atomic<bool> bypassed{false};
    double sampleRate = 44100.0;
    
    // Filter indices in the processor chain
    enum FilterIndex
    {
        LowShelfFilter = 0,
        PeakFilter = 1,
        HighShelfFilter = 2
    };

    void updateFilters();
    void initializeFilters();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQNode)
};