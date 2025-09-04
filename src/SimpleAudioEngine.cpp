#include "SimpleAudioEngine.h"
#include <limits>

SimpleAudioEngine::SimpleAudioEngine() = default;

SimpleAudioEngine::~SimpleAudioEngine()
{
    shutdown();
}

void SimpleAudioEngine::initialize()
{
    if (isInitialized.load())
        return;

    // Initialize audio device manager with input channels for recording
    deviceManager = std::make_unique<juce::AudioDeviceManager>();
    juce::String audioError = deviceManager->initialise(2, 2, nullptr, true); // 2 input, 2 output channels
    
    if (audioError.isNotEmpty())
    {
        // For now, just continue - we'll handle errors gracefully
        DBG("Audio device error: " + audioError);
    }

    // Set up audio format manager for file loading
    formatManager = std::make_unique<juce::AudioFormatManager>();
    formatManager->registerBasicFormats(); // WAV, AIFF built into JUCE

    // Set up transport source
    transportSource = std::make_unique<juce::AudioTransportSource>();
    transportSource->addChangeListener(this);

    // Set audio callback
    deviceManager->addAudioCallback(this);

    isInitialized.store(true);
}

void SimpleAudioEngine::shutdown()
{
    if (!isInitialized.load())
        return;

    stop();
    
    if (deviceManager)
    {
        deviceManager->removeAudioCallback(this);
        deviceManager->closeAudioDevice();
    }

    if (transportSource)
    {
        transportSource->removeChangeListener(this);
        transportSource->setSource(nullptr);
    }

    readerSource = nullptr;
    transportSource = nullptr;
    formatManager = nullptr;
    deviceManager = nullptr;

    fileLoaded.store(false);
    isInitialized.store(false);
}

bool SimpleAudioEngine::loadAudioFile(const juce::File& file)
{
    if (!isInitialized.load() || !formatManager)
        return false;

    stop();

    // Create reader for the file
    auto* reader = formatManager->createReaderFor(file);
    
    if (reader == nullptr)
        return false;

    // Store filename
    currentFileName = file.getFileName();

    // Create reader source
    readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

    // For JUCE 6.0, directly use reader source for simplicity
    transportSource->setSource(readerSource.get(), 0, nullptr);

    // Prepare with current sample rate
    if (auto* device = deviceManager->getCurrentAudioDevice())
    {
        sampleRate = device->getCurrentSampleRate();
        transportSource->prepareToPlay(1024, sampleRate);
    }

    fileLoaded.store(true);

    // Set default loop to full file
    loopStartSeconds.store(0.0);
    loopEndSeconds.store(reader->lengthInSamples / reader->sampleRate);

    // Perform initial beat analysis with default BPM
    performBeatAnalysis();

    return true;
}

void SimpleAudioEngine::closeAudioFile()
{
    stop();
    
    if (transportSource)
        transportSource->setSource(nullptr);
    
    readerSource = nullptr;
    
    fileLoaded.store(false);
    currentFileName.clear();
}

bool SimpleAudioEngine::isFileLoaded() const
{
    return fileLoaded.load();
}

juce::String SimpleAudioEngine::getCurrentFileName() const
{
    return currentFileName;
}

void SimpleAudioEngine::play()
{
    if (!fileLoaded.load() || !transportSource)
        return;

    transportSource->start();
}

void SimpleAudioEngine::pause()
{
    if (!transportSource)
        return;

    transportSource->stop();
}

void SimpleAudioEngine::stop()
{
    if (!transportSource)
        return;

    transportSource->stop();
    transportSource->setPosition(0.0);
}

bool SimpleAudioEngine::isPlaying() const
{
    if (!transportSource)
        return false;

    return transportSource->isPlaying();
}

void SimpleAudioEngine::setPosition(double seconds)
{
    if (!transportSource)
        return;

    transportSource->setPosition(seconds);
}

double SimpleAudioEngine::getPosition() const
{
    if (!transportSource)
        return 0.0;

    return transportSource->getCurrentPosition();
}

double SimpleAudioEngine::getDuration() const
{
    if (!transportSource)
        return 0.0;

    return transportSource->getLengthInSeconds();
}

void SimpleAudioEngine::setLoopEnabled(bool enabled)
{
    loopEnabled.store(enabled);
}

bool SimpleAudioEngine::getLoopEnabled() const
{
    return loopEnabled.load();
}

void SimpleAudioEngine::setLoopStart(double seconds)
{
    loopStartSeconds.store(juce::jmax(0.0, seconds));
}

void SimpleAudioEngine::setLoopEnd(double seconds)
{
    loopEndSeconds.store(juce::jmax(0.0, seconds));
}

double SimpleAudioEngine::getLoopStart() const
{
    return loopStartSeconds.load();
}

double SimpleAudioEngine::getLoopEnd() const
{
    return loopEndSeconds.load();
}

void SimpleAudioEngine::setTempoRatio(float ratio)
{
    // Clamp tempo ratio to reasonable range
    tempoRatio.store(juce::jlimit(0.25f, 4.0f, ratio));
    // Note: Real tempo changing would require external library like RubberBand
}

void SimpleAudioEngine::setPitchSemitones(int semitones)
{
    // Clamp pitch to reasonable range
    pitchSemitones.store(juce::jlimit(-24, 24, semitones));
    // Note: Real pitch shifting would require external library
}

float SimpleAudioEngine::getTempoRatio() const
{
    return tempoRatio.load();
}

int SimpleAudioEngine::getPitchSemitones() const
{
    return pitchSemitones.load();
}

void SimpleAudioEngine::audioDeviceIOCallback(const float** inputChannelData,
                                             int numInputChannels,
                                             float** outputChannelData,
                                             int numOutputChannels,
                                             int numSamples)
{
    // Process input audio (recording and monitoring)
    if (inputChannelData && numInputChannels > 0)
    {
        processInputAudio(inputChannelData, numInputChannels, numSamples);
    }

    // Clear output first
    for (int i = 0; i < numOutputChannels; ++i)
    {
        if (outputChannelData[i] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
    }

    // Check loop position before getting audio
    checkLoopPosition();

    // Get audio from transport source if available
    if (transportSource && fileLoaded.load())
    {
        juce::AudioSourceChannelInfo channelInfo;
        
        // Create a temporary AudioSampleBuffer from the output data
        juce::AudioSampleBuffer buffer(outputChannelData, numOutputChannels, numSamples);
        channelInfo.buffer = &buffer;
        channelInfo.startSample = 0;
        channelInfo.numSamples = numSamples;

        transportSource->getNextAudioBlock(channelInfo);
    }
    
    // Add input monitoring if enabled
    if (inputMonitoringEnabled.load() && inputChannelData && numInputChannels > 0)
    {
        for (int channel = 0; channel < juce::jmin(numInputChannels, numOutputChannels); ++channel)
        {
            if (inputChannelData[channel] != nullptr && outputChannelData[channel] != nullptr)
            {
                // Mix input to output for monitoring
                juce::FloatVectorOperations::add(outputChannelData[channel], inputChannelData[channel], numSamples);
            }
        }
    }
}

void SimpleAudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    sampleRate = device->getCurrentSampleRate();
    
    if (transportSource)
        transportSource->prepareToPlay(device->getCurrentBufferSizeSamples(), sampleRate);
}

void SimpleAudioEngine::audioDeviceStopped()
{
    if (transportSource)
        transportSource->releaseResources();
}

void SimpleAudioEngine::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    juce::ignoreUnused(source);
    // Handle transport source state changes if needed
}

void SimpleAudioEngine::checkLoopPosition()
{
    if (!loopEnabled.load() || !transportSource || !isPlaying())
        return;

    double currentPos = transportSource->getCurrentPosition();
    double loopStart = loopStartSeconds.load();
    double loopEnd = loopEndSeconds.load();

    if (loopEnd > loopStart && currentPos >= loopEnd)
    {
        transportSource->setPosition(loopStart);
    }
}

void SimpleAudioEngine::startRecording()
{
    if (!isInitialized.load())
        return;
        
    juce::ScopedLock lock(recordingLock);
    
    // Allocate recording buffer for 10 minutes at current sample rate
    int maxRecordingSamples = (int)(sampleRate * 600.0); // 10 minutes
    recordingBuffer = std::make_unique<juce::AudioSampleBuffer>(2, maxRecordingSamples);
    recordingBuffer->clear();
    
    recordingPosition.store(0);
    isRecordingEnabled.store(true);
}

void SimpleAudioEngine::stopRecording()
{
    isRecordingEnabled.store(false);
}

bool SimpleAudioEngine::isRecording() const
{
    return isRecordingEnabled.load();
}

void SimpleAudioEngine::saveRecording(const juce::File& outputFile)
{
    if (!recordingBuffer)
        return;
        
    juce::ScopedLock lock(recordingLock);
    
    int samplesRecorded = recordingPosition.load();
    if (samplesRecorded <= 0)
        return;
        
    // Create a WAV file writer
    auto outputStream = outputFile.createOutputStream();
    if (!outputStream)
        return;
        
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(outputStream.release(), sampleRate, 
                                 recordingBuffer->getNumChannels(), 16, {}, 0));
    
    if (writer)
    {
        writer->writeFromAudioSampleBuffer(*recordingBuffer, 0, samplesRecorded);
    }
}

void SimpleAudioEngine::setInputMonitoring(bool enabled)
{
    inputMonitoringEnabled.store(enabled);
}

bool SimpleAudioEngine::getInputMonitoring() const
{
    return inputMonitoringEnabled.load();
}

void SimpleAudioEngine::processInputAudio(const float** inputChannelData, int numInputChannels, int numSamples)
{
    // Input monitoring - mix input to output
    if (inputMonitoringEnabled.load())
    {
        // This would typically be handled in the main audio callback
        // For now, we'll just process the recording
    }
    
    // Recording
    if (isRecordingEnabled.load() && recordingBuffer)
    {
        juce::ScopedLock lock(recordingLock);
        
        int currentPos = recordingPosition.load();
        int maxSamples = recordingBuffer->getNumSamples();
        
        if (currentPos + numSamples <= maxSamples)
        {
            // Copy input audio to recording buffer
            for (int channel = 0; channel < juce::jmin(numInputChannels, recordingBuffer->getNumChannels()); ++channel)
            {
                if (inputChannelData[channel] != nullptr)
                {
                    recordingBuffer->copyFrom(channel, currentPos, inputChannelData[channel], numSamples);
                }
            }
            
            recordingPosition.store(currentPos + numSamples);
        }
        else
        {
            // Recording buffer full - stop recording
            isRecordingEnabled.store(false);
        }
    }
}

// Advanced loop control implementations
void SimpleAudioEngine::setLoopAPoint()
{
    if (!transportSource)
        return;
        
    double currentPos = transportSource->getCurrentPosition();
    double snappedPos = snapToGrid(currentPos);
    loopStartSeconds.store(snappedPos);
    hasAPoint.store(true);
    
    // Auto-enable looping if we have both A and B points
    if (hasBPoint.load())
    {
        loopEnabled.store(true);
    }
}

void SimpleAudioEngine::setLoopBPoint()
{
    if (!transportSource)
        return;
        
    double currentPos = transportSource->getCurrentPosition();
    double snappedPos = snapToGrid(currentPos);
    loopEndSeconds.store(snappedPos);
    hasBPoint.store(true);
    
    // Auto-enable looping if we have both A and B points
    if (hasAPoint.load())
    {
        loopEnabled.store(true);
    }
}

void SimpleAudioEngine::clearLoop()
{
    loopEnabled.store(false);
    hasAPoint.store(false);
    hasBPoint.store(false);
    
    // Reset to full file loop
    if (transportSource)
    {
        loopStartSeconds.store(0.0);
        loopEndSeconds.store(transportSource->getLengthInSeconds());
    }
}

void SimpleAudioEngine::jogLoopStart(double offsetSeconds)
{
    if (!hasAPoint.load())
        return;
        
    double newStart = loopStartSeconds.load() + offsetSeconds;
    double loopEnd = loopEndSeconds.load();
    
    // Ensure start doesn't go past end
    if (newStart < loopEnd)
    {
        loopStartSeconds.store(juce::jmax(0.0, newStart));
    }
}

void SimpleAudioEngine::jogLoopEnd(double offsetSeconds)
{
    if (!hasBPoint.load() || !transportSource)
        return;
        
    double newEnd = loopEndSeconds.load() + offsetSeconds;
    double loopStart = loopStartSeconds.load();
    double maxLength = transportSource->getLengthInSeconds();
    
    // Ensure end doesn't go past start or file length
    if (newEnd > loopStart)
    {
        loopEndSeconds.store(juce::jmin(maxLength, newEnd));
    }
}

void SimpleAudioEngine::doubleLoopLength()
{
    if (!hasAPoint.load() || !hasBPoint.load() || !transportSource)
        return;
        
    double loopStart = loopStartSeconds.load();
    double loopEnd = loopEndSeconds.load();
    double currentLength = loopEnd - loopStart;
    double maxLength = transportSource->getLengthInSeconds();
    
    double newEnd = loopStart + (currentLength * 2.0);
    if (newEnd <= maxLength)
    {
        loopEndSeconds.store(newEnd);
    }
}

void SimpleAudioEngine::halveLoopLength()
{
    if (!hasAPoint.load() || !hasBPoint.load())
        return;
        
    double loopStart = loopStartSeconds.load();
    double loopEnd = loopEndSeconds.load();
    double currentLength = loopEnd - loopStart;
    
    if (currentLength > 0.1) // Minimum 100ms loop
    {
        double newEnd = loopStart + (currentLength * 0.5);
        loopEndSeconds.store(newEnd);
    }
}

void SimpleAudioEngine::moveLoopRegionBackward()
{
    if (!hasAPoint.load() || !hasBPoint.load())
        return;
        
    double loopStart = loopStartSeconds.load();
    double loopEnd = loopEndSeconds.load();
    double loopLength = loopEnd - loopStart;
    
    if (loopStart - loopLength >= 0.0)
    {
        loopStartSeconds.store(loopStart - loopLength);
        loopEndSeconds.store(loopEnd - loopLength);
    }
}

void SimpleAudioEngine::moveLoopRegionForward()
{
    if (!hasAPoint.load() || !hasBPoint.load() || !transportSource)
        return;
        
    double loopStart = loopStartSeconds.load();
    double loopEnd = loopEndSeconds.load();
    double loopLength = loopEnd - loopStart;
    double maxLength = transportSource->getLengthInSeconds();
    
    if (loopEnd + loopLength <= maxLength)
    {
        loopStartSeconds.store(loopStart + loopLength);
        loopEndSeconds.store(loopEnd + loopLength);
    }
}

void SimpleAudioEngine::setEdgeBleedMs(int milliseconds)
{
    edgeBleedMs.store(juce::jlimit(0, 100, milliseconds)); // 0-100ms range
}

int SimpleAudioEngine::getEdgeBleedMs() const
{
    return edgeBleedMs.load();
}

void SimpleAudioEngine::setSnapToGrid(bool enabled)
{
    snapToGridEnabled.store(enabled);
}

bool SimpleAudioEngine::getSnapToGrid() const
{
    return snapToGridEnabled.load();
}

void SimpleAudioEngine::performBeatAnalysis()
{
    if (!fileLoaded.load() || !readerSource)
        return;
        
    juce::ScopedLock lock(beatAnalysisLock);
    
    // Simple beat detection using energy-based analysis
    // This is a basic implementation - professional tools use FFT and onset detection
    beatPositions.clear();
    
    if (auto* reader = readerSource->getAudioFormatReader())
    {
        double duration = reader->lengthInSamples / reader->sampleRate;
        double currentBPM = bpm.load();
        
        if (currentBPM > 0)
        {
            // Generate grid positions based on current BPM
            double beatInterval = 60.0 / currentBPM; // seconds per beat
            
            for (double pos = 0.0; pos < duration; pos += beatInterval)
            {
                beatPositions.push_back(pos);
            }
        }
    }
}

void SimpleAudioEngine::setBPM(double newBPM)
{
    bpm.store(juce::jlimit(60.0, 200.0, newBPM));
    // Regenerate beat positions with new BPM
    performBeatAnalysis();
}

double SimpleAudioEngine::getBPM() const
{
    return bpm.load();
}

double SimpleAudioEngine::snapToGrid(double seconds) const
{
    if (!snapToGridEnabled.load() || beatPositions.empty())
        return seconds;
        
    juce::ScopedLock lock(beatAnalysisLock);
    
    // Find the closest beat position
    double closestBeat = seconds;
    double minDistance = std::numeric_limits<double>::max();
    
    for (double beatPos : beatPositions)
    {
        double distance = std::abs(seconds - beatPos);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestBeat = beatPos;
        }
    }
    
    return closestBeat;
}

std::vector<double> SimpleAudioEngine::getBeatPositions() const
{
    juce::ScopedLock lock(beatAnalysisLock);
    return beatPositions;
}

// Pedal-style loop recording implementation
void SimpleAudioEngine::toggleLoopRecord()
{
    LoopRecordState currentState = loopRecordState.load();
    
    switch (currentState)
    {
        case LoopRecordState::Idle:
            // Start recording from input
            recordingStartTime = transportSource ? transportSource->getCurrentPosition() : 0.0;
            startRecording();
            loopRecordState.store(LoopRecordState::Recording);
            break;
            
        case LoopRecordState::Recording:
        {
            // Stop recording and immediately start looping
            stopRecording();
            recordingEndTime = transportSource ? transportSource->getCurrentPosition() : 0.0;
            
            // Set loop points with overlap
            double overlapSeconds = loopOverlapMs.load() / 1000.0;
            loopStartSeconds.store(recordingStartTime);
            loopEndSeconds.store(recordingEndTime + overlapSeconds);
            hasAPoint.store(true);
            hasBPoint.store(true);
            loopEnabled.store(true);
            
            // Jump to loop start
            if (transportSource)
            {
                transportSource->setPosition(recordingStartTime);
                transportSource->start();
            }
            
            loopRecordState.store(LoopRecordState::Looping);
            break;
        }
            
        case LoopRecordState::Looping:
            // Clear loop and continue with source
            clearLoop();
            loopRecordState.store(LoopRecordState::Idle);
            break;
    }
}

SimpleAudioEngine::LoopRecordState SimpleAudioEngine::getLoopRecordState() const
{
    return loopRecordState.load();
}

void SimpleAudioEngine::setLoopOverlapMs(int milliseconds)
{
    loopOverlapMs.store(juce::jlimit(0, 300, milliseconds));
}

int SimpleAudioEngine::getLoopOverlapMs() const
{
    return loopOverlapMs.load();
}