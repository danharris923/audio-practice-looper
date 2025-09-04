#pragma once

#include <juce_core/juce_core.h>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

#include "Utils/LockFreeRingBuffer.h"

// Forward declarations for aubio types
extern "C" 
{
    struct aubio_tempo_t;
    struct aubio_onset_t;
    struct fvec_t;
}

struct AnalysisResult
{
    std::vector<double> beats;        // Beat positions in seconds
    std::vector<double> onsets;      // Onset positions in seconds
    double bpm = 0.0;                // Detected BPM
    double confidence = 0.0;         // Detection confidence
    bool isValid = false;            // Whether analysis completed successfully
};

class AnalysisWorker
{
public:
    AnalysisWorker();
    ~AnalysisWorker();

    // Control
    void start(double sampleRate);
    void stop();
    bool isRunning() const;

    // Audio input
    void feedAudioData(const float* audioData, int numSamples, int numChannels);
    
    // Results access
    AnalysisResult getLatestResults() const;
    void clearResults();

    // Settings
    void setAnalysisEnabled(bool enabled);

private:
    // Threading
    std::jthread workerThread;
    std::atomic<bool> shouldStop{false};
    std::atomic<bool> isRunning_{false};
    std::atomic<bool> analysisEnabled{true};
    
    // Audio processing
    double sampleRate = 44100.0;
    static constexpr int ANALYSIS_BUFFER_SIZE = 4096;
    static constexpr int HOP_SIZE = 512;
    static constexpr int WINDOW_SIZE = 1024;
    
    // Audio buffers
    LockFreeRingBuffer<float> analysisBuffer;
    std::vector<float> monoBuffer;
    std::vector<float> downsampledBuffer;
    
    // aubio objects
    aubio_tempo_t* tempoDetector = nullptr;
    aubio_onset_t* onsetDetector = nullptr;
    fvec_t* inputVector = nullptr;
    fvec_t* tempoOutput = nullptr;
    fvec_t* onsetOutput = nullptr;
    
    // Analysis results (protected by mutex)
    mutable std::mutex resultsMutex;
    AnalysisResult currentResults;
    
    // Processing state
    std::atomic<double> currentTimeSeconds{0.0};
    double lastBeatTime = 0.0;
    std::vector<double> recentBeats;
    
    // Methods
    void processAnalysis();
    void initializeAubio();
    void cleanupAubio();
    void analyzeAudioChunk(const float* monoData, int numSamples);
    void downsampleToMono(const float* audioData, int numSamples, int numChannels);
    double calculateBPM(const std::vector<double>& beats);
    void updateResults();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalysisWorker)
};