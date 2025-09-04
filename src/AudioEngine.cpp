#include "AudioEngine.h"
#include "AudioFileSource.h"

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine()
{
    shutdown();
}

void AudioEngine::initialize()
{
    deviceManager = std::make_unique<juce::AudioDeviceManager>();
    processorGraph = std::make_unique<juce::AudioProcessorGraph>();
    
    // Initialize audio device manager with default settings
    juce::String audioError = deviceManager->initialise(0, 2, nullptr, true);
    if (audioError.isNotEmpty())
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon,
                                         "Audio Device Error",
                                         audioError);
        return;
    }

    // Set audio callback
    deviceManager->addAudioCallback(this);

    // Setup parameter smoothers
    tempoSmoother.setSampleRate(44100.0);
    tempoSmoother.setSmoothingTimeMs(50.0f);
    tempoSmoother.setCurrentAndTargetValue(1.0f);

    pitchSmoother.setSampleRate(44100.0);
    pitchSmoother.setSmoothingTimeMs(50.0f);
    pitchSmoother.setCurrentAndTargetValue(0.0f);

    // Initialize analysis worker
    analysisWorker = std::make_unique<AnalysisWorker>();

    setupAudioGraph();
}

void AudioEngine::shutdown()
{
    // Stop analysis worker first
    if (analysisWorker)
    {
        analysisWorker->stop();
        analysisWorker = nullptr;
    }

    if (deviceManager)
    {
        deviceManager->removeAudioCallback(this);
        deviceManager->closeAudioDevice();
        deviceManager = nullptr;
    }

    // Clear graph and nodes
    if (processorGraph)
    {
        processorGraph->clear();
        processorGraph = nullptr;
    }

    // Clear node pointers
    fileSource = nullptr;
    rubberBandNode = nullptr;
    eqNode = nullptr;
}

void AudioEngine::setupAudioGraph()
{
    if (!processorGraph)
        return;

    // Clear existing graph
    processorGraph->clear();

    // Create audio I/O nodes
    audioInputNodeID = processorGraph->addNode(
        std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
            juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode))->nodeID;

    audioOutputNodeID = processorGraph->addNode(
        std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
            juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode))->nodeID;

    // Create processing nodes
    fileSource = std::make_unique<AudioFileSource>();
    fileSourceNodeID = processorGraph->addNode(std::unique_ptr<AudioFileSource>(fileSource.release()))->nodeID;

    rubberBandNode = std::make_unique<RubberBandNode>();
    rubberBandNodeID = processorGraph->addNode(std::unique_ptr<RubberBandNode>(rubberBandNode.release()))->nodeID;

    eqNode = std::make_unique<EQNode>();
    eqNodeID = processorGraph->addNode(std::unique_ptr<EQNode>(eqNode.release()))->nodeID;

    // Connect nodes: FileSource -> RubberBand -> EQ -> Output
    for (int channel = 0; channel < 2; ++channel)
    {
        processorGraph->addConnection({{fileSourceNodeID, channel}, {rubberBandNodeID, channel}});
        processorGraph->addConnection({{rubberBandNodeID, channel}, {eqNodeID, channel}});
        processorGraph->addConnection({{eqNodeID, channel}, {audioOutputNodeID, channel}});
    }
}

bool AudioEngine::loadAudioFile(const juce::File& file)
{
    if (!processorGraph)
        return false;

    // Get the file source node from the graph
    if (auto* node = processorGraph->getNodeForId(fileSourceNodeID))
    {
        if (auto* audioFileSource = dynamic_cast<AudioFileSource*>(node->getProcessor()))
        {
            bool success = audioFileSource->loadFile(file);
            if (success)
            {
                // Update loop points to full file by default
                double totalLength = audioFileSource->getTotalLength();
                setLoopInSeconds(0.0);
                setLoopOutSeconds(totalLength);
                setLoopEnabled(false);
            }
            return success;
        }
    }
    
    return false;
}

void AudioEngine::closeAudioFile()
{
    if (!processorGraph)
        return;

    // Get the file source node from the graph
    if (auto* node = processorGraph->getNodeForId(fileSourceNodeID))
    {
        if (auto* audioFileSource = dynamic_cast<AudioFileSource*>(node->getProcessor()))
        {
            audioFileSource->closeFile();
        }
    }
    
    // Reset playback state
    stop();
}

void AudioEngine::play()
{
    isPlaying_.store(true);
}

void AudioEngine::pause()
{
    isPlaying_.store(false);
}

void AudioEngine::stop()
{
    isPlaying_.store(false);
    // TODO: Reset playback position
}

bool AudioEngine::isPlaying() const
{
    return isPlaying_.load();
}

void AudioEngine::setTempoRatio(float ratio)
{
    tempoRatio.store(juce::jlimit(0.25f, 4.0f, ratio));
    tempoSmoother.setTargetValue(tempoRatio.load());
}

void AudioEngine::setPitchSemitones(int semitones)
{
    pitchSemitones.store(juce::jlimit(-24, 24, semitones));
    pitchSmoother.setTargetValue(static_cast<float>(pitchSemitones.load()));
}

void AudioEngine::setLoopInSeconds(double seconds)
{
    loopInSeconds.store(juce::jmax(0.0, seconds));
    
    // Update the AudioFileSource loop points
    if (auto* node = processorGraph->getNodeForId(fileSourceNodeID))
    {
        if (auto* audioFileSource = dynamic_cast<AudioFileSource*>(node->getProcessor()))
        {
            audioFileSource->setLoopPoints(loopInSeconds.load(), loopOutSeconds.load());
        }
    }
}

void AudioEngine::setLoopOutSeconds(double seconds)
{
    loopOutSeconds.store(juce::jmax(0.0, seconds));
    
    // Update the AudioFileSource loop points
    if (auto* node = processorGraph->getNodeForId(fileSourceNodeID))
    {
        if (auto* audioFileSource = dynamic_cast<AudioFileSource*>(node->getProcessor()))
        {
            audioFileSource->setLoopPoints(loopInSeconds.load(), loopOutSeconds.load());
        }
    }
}

void AudioEngine::setLoopEnabled(bool enabled)
{
    loopEnabled.store(enabled);
    
    // Update the AudioFileSource loop state
    if (auto* node = processorGraph->getNodeForId(fileSourceNodeID))
    {
        if (auto* audioFileSource = dynamic_cast<AudioFileSource*>(node->getProcessor()))
        {
            audioFileSource->setLoopEnabled(enabled);
        }
    }
}

void AudioEngine::updateParameters()
{
    // Update Rubber Band parameters if needed
    float currentTempo = tempoSmoother.getNextValue();
    float currentPitch = pitchSmoother.getNextValue();

    // Get the actual RubberBand node from the graph
    if (auto* node = processorGraph->getNodeForId(rubberBandNodeID))
    {
        if (auto* rubberBand = dynamic_cast<RubberBandNode*>(node->getProcessor()))
        {
            rubberBand->setTimeRatio(currentTempo);
            rubberBand->setPitchScale(std::pow(2.0f, currentPitch / 12.0f));
        }
    }
}

void AudioEngine::audioDeviceIOCallbackWithContext(
    const float* const* inputChannelData,
    int numInputChannels,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(inputChannelData, numInputChannels, context);

    // Update parameters smoothly
    updateParameters();

    // Create audio buffer for processing
    juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
    
    if (processorGraph && isPlaying())
    {
        // Process through the graph
        juce::MidiBuffer midiBuffer;
        processorGraph->processBlock(buffer, midiBuffer);
        
        // Feed processed audio to analysis worker
        if (analysisWorker && numOutputChannels > 0)
        {
            analysisWorker->feedAudioData(buffer.getReadPointer(0), numSamples, 
                                        juce::jmin(numOutputChannels, 2));
        }
    }
    else
    {
        // Clear output if not playing
        buffer.clear();
    }
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device)
    {
        sampleRate = device->getCurrentSampleRate();
        blockSize = device->getCurrentBufferSizeSamples();

        // Update parameter smoothers with new sample rate
        tempoSmoother.setSampleRate(sampleRate);
        pitchSmoother.setSampleRate(sampleRate);

        // Prepare the processor graph
        if (processorGraph)
        {
            processorGraph->setPlayConfigDetails(0, 2, sampleRate, blockSize);
            processorGraph->prepareToPlay(sampleRate, blockSize);
        }
        
        // Start analysis worker
        if (analysisWorker)
        {
            analysisWorker->start(sampleRate);
        }
    }
}

void AudioEngine::audioDeviceStopped()
{
    // Stop analysis worker
    if (analysisWorker)
    {
        analysisWorker->stop();
    }

    if (processorGraph)
    {
        processorGraph->releaseResources();
    }
}

// Analysis methods
AnalysisResult AudioEngine::getAnalysisResults() const
{
    if (analysisWorker)
    {
        return analysisWorker->getLatestResults();
    }
    return AnalysisResult{};
}

void AudioEngine::setAnalysisEnabled(bool enabled)
{
    if (analysisWorker)
    {
        analysisWorker->setAnalysisEnabled(enabled);
    }
}