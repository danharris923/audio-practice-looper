#include "AnalysisWorker.h"

// Include aubio headers
extern "C" 
{
    #include <aubio/aubio.h>
}

#include <algorithm>
#include <cmath>

AnalysisWorker::AnalysisWorker()
    : analysisBuffer(ANALYSIS_BUFFER_SIZE * 4) // Larger buffer for continuous analysis
{
    // Pre-allocate buffers
    monoBuffer.reserve(HOP_SIZE);
    downsampledBuffer.reserve(HOP_SIZE);
    recentBeats.reserve(100); // Keep track of recent beats for BPM calculation
}

AnalysisWorker::~AnalysisWorker()
{
    stop();
}

void AnalysisWorker::start(double sampleRate)
{
    if (isRunning_.load())
        return;
        
    this->sampleRate = sampleRate;
    shouldStop.store(false);
    
    // Clear previous results
    clearResults();
    
    // Start analysis thread
    workerThread = std::jthread([this]() { processAnalysis(); });
    
    juce::Logger::writeToLog("AnalysisWorker: Started analysis at " + 
                            juce::String(sampleRate, 0) + "Hz");
}

void AnalysisWorker::stop()
{
    shouldStop.store(true);
    
    if (workerThread.joinable())
    {
        workerThread.join();
    }
    
    isRunning_.store(false);
    
    // Clean up aubio resources
    cleanupAubio();
}

bool AnalysisWorker::isRunning() const
{
    return isRunning_.load();
}

void AnalysisWorker::feedAudioData(const float* audioData, int numSamples, int numChannels)
{
    if (!analysisEnabled.load() || !isRunning_.load())
        return;

    // Downsample to mono and feed to analysis buffer
    downsampleToMono(audioData, numSamples, numChannels);
    
    // Try to write to ring buffer (non-blocking)
    if (monoBuffer.size() > 0)
    {
        analysisBuffer.write(monoBuffer.data(), monoBuffer.size());
    }
    
    // Update time position
    double timeAdvancement = static_cast<double>(numSamples) / sampleRate;
    currentTimeSeconds.store(currentTimeSeconds.load() + timeAdvancement);
}

void AnalysisWorker::downsampleToMono(const float* audioData, int numSamples, int numChannels)
{
    monoBuffer.resize(numSamples);
    
    if (numChannels == 1)
    {
        // Already mono, just copy
        std::copy(audioData, audioData + numSamples, monoBuffer.begin());
    }
    else
    {
        // Convert to mono by averaging channels
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float sum = 0.0f;
            for (int channel = 0; channel < numChannels; ++channel)
            {
                sum += audioData[sample * numChannels + channel];
            }
            monoBuffer[sample] = sum / static_cast<float>(numChannels);
        }
    }
}

void AnalysisWorker::processAnalysis()
{
    isRunning_.store(true);
    
    // Initialize aubio
    initializeAubio();
    
    if (!tempoDetector || !onsetDetector)
    {
        juce::Logger::writeToLog("AnalysisWorker: Failed to initialize aubio");
        isRunning_.store(false);
        return;
    }
    
    std::vector<float> processingBuffer(HOP_SIZE);
    
    while (!shouldStop.load())
    {
        // Check if we have enough data to process
        if (analysisBuffer.available() >= HOP_SIZE)
        {
            // Read audio data from ring buffer
            if (analysisBuffer.read(processingBuffer.data(), HOP_SIZE))
            {
                analyzeAudioChunk(processingBuffer.data(), HOP_SIZE);
            }
        }
        else
        {
            // No data available, sleep briefly
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    
    isRunning_.store(false);
}

void AnalysisWorker::initializeAubio()
{
    cleanupAubio(); // Clean up any existing objects
    
    // Create aubio tempo detector
    tempoDetector = new_aubio_tempo("default", WINDOW_SIZE, HOP_SIZE, static_cast<uint_t>(sampleRate));
    if (!tempoDetector)
    {
        juce::Logger::writeToLog("AnalysisWorker: Failed to create tempo detector");
        return;
    }
    
    // Create aubio onset detector  
    onsetDetector = new_aubio_onset("default", WINDOW_SIZE, HOP_SIZE, static_cast<uint_t>(sampleRate));
    if (!onsetDetector)
    {
        juce::Logger::writeToLog("AnalysisWorker: Failed to create onset detector");
        return;
    }
    
    // Create input and output vectors
    inputVector = new_fvec(HOP_SIZE);
    tempoOutput = new_fvec(2); // tempo output (beat detection)
    onsetOutput = new_fvec(1); // onset output (onset detection)
    
    if (!inputVector || !tempoOutput || !onsetOutput)
    {
        juce::Logger::writeToLog("AnalysisWorker: Failed to create aubio vectors");
        cleanupAubio();
        return;
    }
    
    juce::Logger::writeToLog("AnalysisWorker: aubio initialized successfully");
}

void AnalysisWorker::cleanupAubio()
{
    if (tempoDetector)
    {
        del_aubio_tempo(tempoDetector);
        tempoDetector = nullptr;
    }
    
    if (onsetDetector)
    {
        del_aubio_onset(onsetDetector);
        onsetDetector = nullptr;
    }
    
    if (inputVector)
    {
        del_fvec(inputVector);
        inputVector = nullptr;
    }
    
    if (tempoOutput)
    {
        del_fvec(tempoOutput);
        tempoOutput = nullptr;
    }
    
    if (onsetOutput)
    {
        del_fvec(onsetOutput);
        onsetOutput = nullptr;
    }
}

void AnalysisWorker::analyzeAudioChunk(const float* monoData, int numSamples)
{
    if (!tempoDetector || !onsetDetector || !inputVector || numSamples != HOP_SIZE)
        return;
    
    // Copy audio data to aubio input vector
    for (int i = 0; i < numSamples; ++i)
    {
        fvec_set_sample(inputVector, monoData[i], i);
    }
    
    // Process tempo detection
    aubio_tempo_do(tempoDetector, inputVector, tempoOutput);
    
    // Check for beat detection
    if (fvec_get_sample(tempoOutput, 0) != 0.0f)
    {
        double currentTime = currentTimeSeconds.load() - (static_cast<double>(HOP_SIZE) / sampleRate);
        
        std::lock_guard<std::mutex> lock(resultsMutex);
        currentResults.beats.push_back(currentTime);
        recentBeats.push_back(currentTime);
        lastBeatTime = currentTime;
        
        // Keep only recent beats for BPM calculation (last 10 seconds)
        double cutoffTime = currentTime - 10.0;
        recentBeats.erase(
            std::remove_if(recentBeats.begin(), recentBeats.end(),
                          [cutoffTime](double beatTime) { return beatTime < cutoffTime; }),
            recentBeats.end());
    }
    
    // Process onset detection
    aubio_onset_do(onsetDetector, inputVector, onsetOutput);
    
    // Check for onset detection
    if (fvec_get_sample(onsetOutput, 0) != 0.0f)
    {
        double currentTime = currentTimeSeconds.load() - (static_cast<double>(HOP_SIZE) / sampleRate);
        
        std::lock_guard<std::mutex> lock(resultsMutex);
        currentResults.onsets.push_back(currentTime);
    }
    
    // Periodically update BPM calculation
    static int updateCounter = 0;
    if (++updateCounter >= 10) // Update every 10 chunks
    {
        updateCounter = 0;
        updateResults();
    }
}

void AnalysisWorker::updateResults()
{
    std::lock_guard<std::mutex> lock(resultsMutex);
    
    // Calculate BPM from recent beats
    currentResults.bpm = calculateBPM(recentBeats);
    
    // Calculate confidence based on beat regularity
    if (recentBeats.size() >= 4)
    {
        std::vector<double> intervals;
        for (size_t i = 1; i < recentBeats.size(); ++i)
        {
            intervals.push_back(recentBeats[i] - recentBeats[i-1]);
        }
        
        // Calculate standard deviation of intervals
        double mean = 0.0;
        for (double interval : intervals)
        {
            mean += interval;
        }
        mean /= intervals.size();
        
        double variance = 0.0;
        for (double interval : intervals)
        {
            variance += (interval - mean) * (interval - mean);
        }
        variance /= intervals.size();
        
        double stddev = std::sqrt(variance);
        
        // Lower standard deviation = higher confidence
        currentResults.confidence = std::max(0.0, 1.0 - (stddev / mean));
        currentResults.isValid = true;
    }
    else
    {
        currentResults.confidence = 0.0;
        currentResults.isValid = false;
    }
}

double AnalysisWorker::calculateBPM(const std::vector<double>& beats)
{
    if (beats.size() < 2)
        return 0.0;
    
    // Calculate intervals between beats
    std::vector<double> intervals;
    for (size_t i = 1; i < beats.size(); ++i)
    {
        intervals.push_back(beats[i] - beats[i-1]);
    }
    
    // Calculate median interval (more robust than mean)
    std::sort(intervals.begin(), intervals.end());
    double medianInterval;
    
    if (intervals.size() % 2 == 0)
    {
        medianInterval = (intervals[intervals.size()/2 - 1] + intervals[intervals.size()/2]) / 2.0;
    }
    else
    {
        medianInterval = intervals[intervals.size()/2];
    }
    
    // Convert interval to BPM
    if (medianInterval > 0.0)
    {
        return 60.0 / medianInterval;
    }
    
    return 0.0;
}

AnalysisResult AnalysisWorker::getLatestResults() const
{
    std::lock_guard<std::mutex> lock(resultsMutex);
    return currentResults; // Copy
}

void AnalysisWorker::clearResults()
{
    std::lock_guard<std::mutex> lock(resultsMutex);
    currentResults = AnalysisResult{};
    recentBeats.clear();
    currentTimeSeconds.store(0.0);
}

void AnalysisWorker::setAnalysisEnabled(bool enabled)
{
    analysisEnabled.store(enabled);
}