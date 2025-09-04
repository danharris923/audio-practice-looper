#include "EQNode.h"

EQNode::EQNode()
{
    // Initialize default EQ parameters
    
    // Low shelf: 80Hz, 0dB gain
    lowShelf.frequency.store(80.0f);
    lowShelf.gain.store(0.0f);
    
    // Peak: 1kHz, 0dB gain, Q = 0.707
    peak.frequency.store(1000.0f);
    peak.gain.store(0.0f);
    peak.q.store(0.707f);
    
    // High shelf: 8kHz, 0dB gain
    highShelf.frequency.store(8000.0f);
    highShelf.gain.store(0.0f);
    
    // Initialize smoothers with neutral values
    lowShelfGainSmoother.setCurrentAndTargetValue(0.0f);
    lowShelfFreqSmoother.setCurrentAndTargetValue(80.0f);
    peakGainSmoother.setCurrentAndTargetValue(0.0f);
    peakFreqSmoother.setCurrentAndTargetValue(1000.0f);
    peakQSmoother.setCurrentAndTargetValue(0.707f);
    highShelfGainSmoother.setCurrentAndTargetValue(0.0f);
    highShelfFreqSmoother.setCurrentAndTargetValue(8000.0f);
}

EQNode::~EQNode() = default;

void EQNode::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    
    // Prepare the processor chain
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(juce::jmin(getTotalNumInputChannels(), 2));
    
    processorChain.prepare(spec);
    
    // Setup parameter smoothers
    const float smoothingTimeMs = 20.0f; // Quick response for EQ
    
    lowShelfGainSmoother.setSampleRate(sampleRate);
    lowShelfGainSmoother.setSmoothingTimeMs(smoothingTimeMs);
    
    lowShelfFreqSmoother.setSampleRate(sampleRate);
    lowShelfFreqSmoother.setSmoothingTimeMs(smoothingTimeMs);
    
    peakGainSmoother.setSampleRate(sampleRate);
    peakGainSmoother.setSmoothingTimeMs(smoothingTimeMs);
    
    peakFreqSmoother.setSampleRate(sampleRate);
    peakFreqSmoother.setSmoothingTimeMs(smoothingTimeMs);
    
    peakQSmoother.setSampleRate(sampleRate);
    peakQSmoother.setSmoothingTimeMs(smoothingTimeMs);
    
    highShelfGainSmoother.setSampleRate(sampleRate);
    highShelfGainSmoother.setSmoothingTimeMs(smoothingTimeMs);
    
    highShelfFreqSmoother.setSampleRate(sampleRate);
    highShelfFreqSmoother.setSmoothingTimeMs(smoothingTimeMs);
    
    // Initialize filters
    initializeFilters();
}

void EQNode::releaseResources()
{
    processorChain.reset();
}

void EQNode::initializeFilters()
{
    if (sampleRate <= 0)
        return;
        
    // Initialize Low Shelf Filter (80Hz default)
    auto& lowShelfFilter = processorChain.get<LowShelfFilter>();
    *lowShelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate,
        lowShelf.frequency.load(),
        1.0f, // Q factor (not used for shelf filters)
        juce::Decibels::decibelsToGain(lowShelf.gain.load())
    );
    
    // Initialize Peak Filter (1kHz default)
    auto& peakFilter = processorChain.get<PeakFilter>();
    *peakFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate,
        peak.frequency.load(),
        peak.q.load(),
        juce::Decibels::decibelsToGain(peak.gain.load())
    );
    
    // Initialize High Shelf Filter (8kHz default)
    auto& highShelfFilter = processorChain.get<HighShelfFilter>();
    *highShelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate,
        highShelf.frequency.load(),
        1.0f, // Q factor (not used for shelf filters)
        juce::Decibels::decibelsToGain(highShelf.gain.load())
    );
}

void EQNode::updateFilters()
{
    // Get smoothed parameter values
    float lowShelfGain = lowShelfGainSmoother.getNextValue();
    float lowShelfFreq = lowShelfFreqSmoother.getNextValue();
    
    float peakGain = peakGainSmoother.getNextValue();
    float peakFreq = peakFreqSmoother.getNextValue();
    float peakQ = peakQSmoother.getNextValue();
    
    float highShelfGain = highShelfGainSmoother.getNextValue();
    float highShelfFreq = highShelfFreqSmoother.getNextValue();
    
    // Update Low Shelf Filter
    auto& lowShelfFilter = processorChain.get<LowShelfFilter>();
    *lowShelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate,
        juce::jlimit(20.0f, static_cast<float>(sampleRate * 0.4), lowShelfFreq),
        1.0f,
        juce::Decibels::decibelsToGain(juce::jlimit(-24.0f, 24.0f, lowShelfGain))
    );
    
    // Update Peak Filter
    auto& peakFilter = processorChain.get<PeakFilter>();
    *peakFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate,
        juce::jlimit(20.0f, static_cast<float>(sampleRate * 0.4), peakFreq),
        juce::jlimit(0.1f, 10.0f, peakQ),
        juce::Decibels::decibelsToGain(juce::jlimit(-24.0f, 24.0f, peakGain))
    );
    
    // Update High Shelf Filter
    auto& highShelfFilter = processorChain.get<HighShelfFilter>();
    *highShelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate,
        juce::jlimit(20.0f, static_cast<float>(sampleRate * 0.4), highShelfFreq),
        1.0f,
        juce::Decibels::decibelsToGain(juce::jlimit(-24.0f, 24.0f, highShelfGain))
    );
}

void EQNode::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Clear any unused output channels
    for (int channel = numChannels; channel < getTotalNumOutputChannels(); ++channel)
    {
        buffer.clear(channel, 0, numSamples);
    }
    
    // If bypassed, just pass through
    if (bypassed.load() || numSamples == 0)
    {
        return;
    }
    
    // Update filter parameters smoothly
    updateFilters();
    
    // Process the audio through the EQ chain
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    processorChain.process(context);
}

// Control methods implementation
void EQNode::setLowShelfGain(float gainDb)
{
    float clampedGain = juce::jlimit(-24.0f, 24.0f, gainDb);
    lowShelf.gain.store(clampedGain);
    lowShelfGainSmoother.setTargetValue(clampedGain);
}

void EQNode::setLowShelfFrequency(float frequency)
{
    float clampedFreq = juce::jlimit(20.0f, 500.0f, frequency);
    lowShelf.frequency.store(clampedFreq);
    lowShelfFreqSmoother.setTargetValue(clampedFreq);
}

void EQNode::setPeakGain(float gainDb)
{
    float clampedGain = juce::jlimit(-24.0f, 24.0f, gainDb);
    peak.gain.store(clampedGain);
    peakGainSmoother.setTargetValue(clampedGain);
}

void EQNode::setPeakFrequency(float frequency)
{
    float clampedFreq = juce::jlimit(200.0f, 8000.0f, frequency);
    peak.frequency.store(clampedFreq);
    peakFreqSmoother.setTargetValue(clampedFreq);
}

void EQNode::setPeakQ(float q)
{
    float clampedQ = juce::jlimit(0.1f, 10.0f, q);
    peak.q.store(clampedQ);
    peakQSmoother.setTargetValue(clampedQ);
}

void EQNode::setHighShelfGain(float gainDb)
{
    float clampedGain = juce::jlimit(-24.0f, 24.0f, gainDb);
    highShelf.gain.store(clampedGain);
    highShelfGainSmoother.setTargetValue(clampedGain);
}

void EQNode::setHighShelfFrequency(float frequency)
{
    float clampedFreq = juce::jlimit(2000.0f, 20000.0f, frequency);
    highShelf.frequency.store(clampedFreq);
    highShelfFreqSmoother.setTargetValue(clampedFreq);
}

void EQNode::setBypassEnabled(bool bypass)
{
    bypassed.store(bypass);
}