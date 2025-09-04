#include "RubberBandNode.h"

// Include Rubber Band headers
#include <rubberband/RubberBandStretcher.h>

RubberBandNode::RubberBandNode()
{
    // Initialize parameter smoothers with default values
    timeRatioSmoother.setCurrentAndTargetValue(1.0f);
    pitchScaleSmoother.setCurrentAndTargetValue(1.0f);
}

RubberBandNode::~RubberBandNode()
{
    releaseResources();
}

void RubberBandNode::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    this->blockSize = samplesPerBlock;
    this->numChannels = juce::jmin(getTotalNumInputChannels(), 2); // Limit to stereo

    // Setup parameter smoothers
    timeRatioSmoother.setSampleRate(sampleRate);
    timeRatioSmoother.setSmoothingTimeMs(50.0f); // 50ms smoothing
    
    pitchScaleSmoother.setSampleRate(sampleRate);
    pitchScaleSmoother.setSmoothingTimeMs(50.0f);

    // Initialize Rubber Band stretcher
    initializeStretcher();

    // Prepare buffers
    tempBuffer.resize(blockSize * numChannels);
    inputPointers.resize(numChannels);
    outputPointers.resize(numChannels);
}

void RubberBandNode::initializeStretcher()
{
    if (sampleRate <= 0 || numChannels <= 0)
        return;

    // Create stretcher with real-time mode for low latency
    RubberBand::RubberBandStretcher::Options options = 
        RubberBand::RubberBandStretcher::OptionProcessRealTime |
        RubberBand::RubberBandStretcher::OptionStretchElastic |
        RubberBand::RubberBandStretcher::OptionTransientsCrisp |
        RubberBand::RubberBandStretcher::OptionDetectorCompound |
        RubberBand::RubberBandStretcher::OptionPhaseLaminar |
        RubberBand::RubberBandStretcher::OptionThreadingNever |
        RubberBand::RubberBandStretcher::OptionWindowShort |
        RubberBand::RubberBandStretcher::OptionSmoothingOff;

    stretcher = std::make_unique<RubberBand::RubberBandStretcher>(
        static_cast<size_t>(sampleRate),
        static_cast<size_t>(numChannels),
        options,
        1.0, // Initial time ratio
        1.0  // Initial pitch scale
    );

    // Set preferred block size for better performance
    stretcher->setMaxProcessSize(static_cast<size_t>(blockSize));
    
    juce::Logger::writeToLog("RubberBandNode: Initialized stretcher (" + 
                            juce::String(sampleRate, 0) + "Hz, " + 
                            juce::String(numChannels) + " channels)");
}

void RubberBandNode::releaseResources()
{
    stretcher = nullptr;
    tempBuffer.clear();
    inputPointers.clear();
    outputPointers.clear();
}

void RubberBandNode::updateParameters()
{
    // Get smoothed parameter values
    float currentTimeRatio = timeRatioSmoother.getNextValue();
    float currentPitchScale = pitchScaleSmoother.getNextValue();

    // Update stretcher parameters if they have changed
    if (stretcher)
    {
        stretcher->setTimeRatio(currentTimeRatio);
        stretcher->setPitchScale(currentPitchScale);
    }
}

void RubberBandNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numInputChannels = buffer.getNumChannels();

    // Clear any unused output channels
    for (int channel = numInputChannels; channel < getTotalNumOutputChannels(); ++channel)
    {
        buffer.clear(channel, 0, numSamples);
    }

    // If no stretcher or no channels, just pass through
    if (!stretcher || numInputChannels == 0 || numSamples == 0)
    {
        return;
    }

    // Update parameters smoothly
    updateParameters();

    // Prepare input pointers
    for (int channel = 0; channel < juce::jmin(numInputChannels, numChannels); ++channel)
    {
        inputPointers[channel] = buffer.getReadPointer(channel);
    }

    // Process audio through Rubber Band
    stretcher->process(inputPointers.data(), 
                      static_cast<size_t>(numSamples), 
                      false); // false = more input expected

    // Retrieve processed samples
    int available = static_cast<int>(stretcher->available());
    if (available > 0)
    {
        // Limit to buffer size
        int samplesToRetrieve = juce::jmin(available, numSamples);
        
        // Prepare output pointers
        for (int channel = 0; channel < juce::jmin(numInputChannels, numChannels); ++channel)
        {
            outputPointers[channel] = buffer.getWritePointer(channel);
        }

        // Retrieve samples from stretcher
        size_t retrieved = stretcher->retrieve(outputPointers.data(), 
                                              static_cast<size_t>(samplesToRetrieve));

        // Clear any remaining samples in the buffer if we retrieved fewer
        if (static_cast<int>(retrieved) < numSamples)
        {
            for (int channel = 0; channel < numInputChannels; ++channel)
            {
                buffer.clear(channel, static_cast<int>(retrieved), 
                           numSamples - static_cast<int>(retrieved));
            }
        }
    }
    else
    {
        // No samples available, clear the buffer
        buffer.clear();
    }
}

void RubberBandNode::setTimeRatio(float ratio)
{
    // Clamp to reasonable range: 0.25x to 4x speed
    float clampedRatio = juce::jlimit(0.25f, 4.0f, ratio);
    timeRatio.store(clampedRatio);
    timeRatioSmoother.setTargetValue(clampedRatio);
}

void RubberBandNode::setPitchScale(float scale)
{
    // Clamp pitch scale to reasonable range (±2 octaves)
    float clampedScale = juce::jlimit(0.25f, 4.0f, scale);
    pitchScale.store(clampedScale);
    pitchScaleSmoother.setTargetValue(clampedScale);
}