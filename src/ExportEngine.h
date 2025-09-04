#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include <atomic>
#include <memory>
#include <thread>

#include "AudioEngine.h"

class ExportEngine
{
public:
    struct ExportSettings
    {
        juce::File outputFile;
        double startTimeSeconds = 0.0;
        double endTimeSeconds = 0.0;
        int sampleRate = 44100;
        int bitDepth = 16;  // 16, 24, or 32
        int numChannels = 2;
        
        // Export options
        bool applyTimeStretching = true;
        bool applyPitchShifting = true;
        bool applyEQ = true;
        bool exportLoopOnly = false;
        int loopRepetitions = 1;
        
        // Quality settings
        double quality = 0.5;  // 0.0 to 1.0 for format-specific quality
        
        // Fade options
        bool fadeIn = false;
        bool fadeOut = false;
        double fadeInDuration = 1.0;
        double fadeOutDuration = 1.0;
    };
    
    enum class ExportFormat
    {
        WAV,
        MP3,
        FLAC,
        OGG
    };

    ExportEngine();
    ~ExportEngine();

    // Export operations
    bool startExport(const ExportSettings& settings, ExportFormat format);
    void cancelExport();
    bool isExporting() const { return isExporting_.load(); }
    
    // Progress tracking
    double getExportProgress() const { return exportProgress_.load(); }
    std::string getCurrentOperation() const;
    
    // Callbacks
    std::function<void(bool success, const std::string& message)> onExportComplete;
    std::function<void(double progress)> onProgressUpdate;
    std::function<void(const std::string& operation)> onOperationUpdate;
    
    // Format support
    static std::vector<ExportFormat> getSupportedFormats();
    static std::string getFormatExtension(ExportFormat format);
    static std::string getFormatDescription(ExportFormat format);
    
    // Estimate export time and file size
    double estimateExportTime(const ExportSettings& settings) const;
    int64_t estimateFileSize(const ExportSettings& settings, ExportFormat format) const;

private:
    std::atomic<bool> isExporting_{false};
    std::atomic<bool> shouldCancel_{false};
    std::atomic<double> exportProgress_{0.0};
    
    mutable std::mutex operationMutex_;
    std::string currentOperation_;
    
    std::unique_ptr<std::thread> exportThread_;
    
    // Export processing
    void performExport(const ExportSettings& settings, ExportFormat format);
    
    // Audio processing chain
    bool setupProcessingChain(const ExportSettings& settings);
    void processAudioBlock(juce::AudioBuffer<float>& buffer, 
                          const ExportSettings& settings);
    
    // Format-specific writers
    std::unique_ptr<juce::AudioFormatWriter> createWriter(
        const ExportSettings& settings, 
        ExportFormat format, 
        juce::OutputStream* outputStream);
    
    // Processing components (mirrored from AudioEngine but for offline processing)
    std::unique_ptr<juce::AudioFormatManager> formatManager_;
    std::unique_ptr<juce::AudioProcessorGraph> processingGraph_;
    std::unique_ptr<class OfflineAudioFileSource> offlineSource_;
    std::unique_ptr<class OfflineRubberBandNode> offlineRubberBand_;
    std::unique_ptr<class OfflineEQNode> offlineEQ_;
    
    // Helper methods
    void updateProgress(double progress);
    void updateOperation(const std::string& operation);
    bool loadSourceAudio(const juce::File& sourceFile);
    void applyFades(juce::AudioBuffer<float>& buffer, 
                   const ExportSettings& settings, 
                   double currentTimeSeconds);
    
    // Format detection
    ExportFormat detectFormatFromFile(const juce::File& file) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExportEngine)
};