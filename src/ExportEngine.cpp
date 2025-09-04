#include "ExportEngine.h"
#include "AudioFileSource.h"
#include "RubberBandNode.h"
#include "EQNode.h"
#include <juce_audio_formats/juce_audio_formats.h>

ExportEngine::ExportEngine()
{
    formatManager_ = std::make_unique<juce::AudioFormatManager>();
    formatManager_->registerBasicFormats();
    
    // Register additional formats if available
#if JUCE_USE_LAME_AUDIO_FORMAT
    formatManager_->registerFormat(new juce::LAMEEncoderAudioFormat(), false);
#endif

#if JUCE_USE_OGGVORBIS
    formatManager_->registerFormat(new juce::OggVorbisAudioFormat(), false);
#endif

#if JUCE_USE_FLAC
    formatManager_->registerFormat(new juce::FlacAudioFormat(), false);
#endif
}

ExportEngine::~ExportEngine()
{
    cancelExport();
}

bool ExportEngine::startExport(const ExportSettings& settings, ExportFormat format)
{
    if (isExporting_.load())
        return false;
    
    // Validate settings
    if (!settings.outputFile.getParentDirectory().exists())
    {
        if (onExportComplete)
            onExportComplete(false, "Output directory does not exist");
        return false;
    }
    
    if (settings.endTimeSeconds <= settings.startTimeSeconds)
    {
        if (onExportComplete)
            onExportComplete(false, "Invalid time range");
        return false;
    }
    
    shouldCancel_.store(false);
    isExporting_.store(true);
    exportProgress_.store(0.0);
    
    exportThread_ = std::make_unique<std::thread>(
        &ExportEngine::performExport, this, settings, format);
    
    return true;
}

void ExportEngine::cancelExport()
{
    shouldCancel_.store(true);
    
    if (exportThread_ && exportThread_->joinable())
    {
        exportThread_->join();
        exportThread_.reset();
    }
    
    isExporting_.store(false);
}

std::string ExportEngine::getCurrentOperation() const
{
    std::lock_guard<std::mutex> lock(operationMutex_);
    return currentOperation_;
}

std::vector<ExportEngine::ExportFormat> ExportEngine::getSupportedFormats()
{
    std::vector<ExportFormat> formats;
    formats.push_back(ExportFormat::WAV);
    
#if JUCE_USE_LAME_AUDIO_FORMAT
    formats.push_back(ExportFormat::MP3);
#endif

#if JUCE_USE_FLAC
    formats.push_back(ExportFormat::FLAC);
#endif

#if JUCE_USE_OGGVORBIS
    formats.push_back(ExportFormat::OGG);
#endif

    return formats;
}

std::string ExportEngine::getFormatExtension(ExportFormat format)
{
    switch (format)
    {
        case ExportFormat::WAV: return ".wav";
        case ExportFormat::MP3: return ".mp3";
        case ExportFormat::FLAC: return ".flac";
        case ExportFormat::OGG: return ".ogg";
        default: return ".wav";
    }
}

std::string ExportEngine::getFormatDescription(ExportFormat format)
{
    switch (format)
    {
        case ExportFormat::WAV: return "WAV (Uncompressed)";
        case ExportFormat::MP3: return "MP3 (Compressed)";
        case ExportFormat::FLAC: return "FLAC (Lossless)";
        case ExportFormat::OGG: return "OGG Vorbis (Compressed)";
        default: return "Unknown";
    }
}

double ExportEngine::estimateExportTime(const ExportSettings& settings) const
{
    double duration = settings.endTimeSeconds - settings.startTimeSeconds;
    if (settings.exportLoopOnly && settings.loopRepetitions > 1)
        duration *= settings.loopRepetitions;
    
    // Rough estimate: processing is typically 2-10x faster than real-time
    // depending on effects and system performance
    double processingSpeedMultiplier = 5.0;
    if (settings.applyTimeStretching || settings.applyPitchShifting)
        processingSpeedMultiplier *= 0.5; // Rubber Band is computationally expensive
    
    return duration / processingSpeedMultiplier;
}

int64_t ExportEngine::estimateFileSize(const ExportSettings& settings, ExportFormat format) const
{
    double duration = settings.endTimeSeconds - settings.startTimeSeconds;
    if (settings.exportLoopOnly && settings.loopRepetitions > 1)
        duration *= settings.loopRepetitions;
    
    int64_t bytesPerSecond;
    
    switch (format)
    {
        case ExportFormat::WAV:
            bytesPerSecond = settings.sampleRate * settings.numChannels * (settings.bitDepth / 8);
            return static_cast<int64_t>(duration * bytesPerSecond);
            
        case ExportFormat::MP3:
            // Assume ~128 kbps average bitrate
            return static_cast<int64_t>(duration * 128 * 1000 / 8);
            
        case ExportFormat::FLAC:
            // FLAC typically compresses to ~60% of uncompressed size
            bytesPerSecond = settings.sampleRate * settings.numChannels * (settings.bitDepth / 8);
            return static_cast<int64_t>(duration * bytesPerSecond * 0.6);
            
        case ExportFormat::OGG:
            // Assume ~160 kbps average bitrate for good quality
            return static_cast<int64_t>(duration * 160 * 1000 / 8);
    }
    
    return 0;
}

void ExportEngine::performExport(const ExportSettings& settings, ExportFormat format)
{
    bool success = false;
    std::string errorMessage;
    
    try
    {
        updateOperation("Initializing export...");
        updateProgress(0.0);
        
        // Create output stream
        auto outputStream = std::make_unique<juce::FileOutputStream>(settings.outputFile);
        if (!outputStream->openedOk())
        {
            errorMessage = "Failed to create output file";
            throw std::runtime_error(errorMessage);
        }
        
        // Create audio writer
        auto writer = createWriter(settings, format, outputStream.get());
        if (!writer)
        {
            errorMessage = "Failed to create audio writer for format";
            throw std::runtime_error(errorMessage);
        }
        
        updateOperation("Loading source audio...");
        updateProgress(0.1);
        
        // TODO: Load source audio - this would need integration with the main AudioEngine
        // For now, we'll simulate the export process
        
        updateOperation("Processing audio...");
        
        double duration = settings.endTimeSeconds - settings.startTimeSeconds;
        int totalSamples = static_cast<int>(duration * settings.sampleRate);
        int blockSize = 512;
        int processedSamples = 0;
        
        juce::AudioBuffer<float> processingBuffer(settings.numChannels, blockSize);
        
        while (processedSamples < totalSamples && !shouldCancel_.load())
        {
            int samplesToProcess = juce::jmin(blockSize, totalSamples - processedSamples);
            
            // Clear buffer for this block
            processingBuffer.clear();
            
            // TODO: Fill buffer with processed audio from the audio engine
            // This would involve:
            // 1. Reading from the source file
            // 2. Applying time stretching if enabled
            // 3. Applying pitch shifting if enabled  
            // 4. Applying EQ if enabled
            // 5. Applying fades if enabled
            
            // For now, generate silence as placeholder
            processingBuffer.setSize(settings.numChannels, samplesToProcess, false, false, true);
            
            // Apply fades if requested
            if (settings.fadeIn || settings.fadeOut)
            {
                double currentTime = settings.startTimeSeconds + 
                                   (static_cast<double>(processedSamples) / settings.sampleRate);
                applyFades(processingBuffer, settings, currentTime);
            }
            
            // Write to file
            if (!writer->writeFromAudioSampleBuffer(processingBuffer, 0, samplesToProcess))
            {
                errorMessage = "Failed to write audio data";
                throw std::runtime_error(errorMessage);
            }
            
            processedSamples += samplesToProcess;
            
            // Update progress
            double progress = 0.1 + 0.8 * (static_cast<double>(processedSamples) / totalSamples);
            updateProgress(progress);
            
            // Allow other threads to run
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        if (shouldCancel_.load())
        {
            errorMessage = "Export cancelled by user";
            throw std::runtime_error(errorMessage);
        }
        
        updateOperation("Finalizing file...");
        updateProgress(0.95);
        
        // Finalize the writer
        writer.reset();
        outputStream.reset();
        
        updateOperation("Export complete");
        updateProgress(1.0);
        success = true;
    }
    catch (const std::exception& e)
    {
        errorMessage = e.what();
    }
    
    isExporting_.store(false);
    
    if (onExportComplete)
        onExportComplete(success, success ? "Export completed successfully" : errorMessage);
}

std::unique_ptr<juce::AudioFormatWriter> ExportEngine::createWriter(
    const ExportSettings& settings, 
    ExportFormat format, 
    juce::OutputStream* outputStream)
{
    juce::AudioFormat* audioFormat = nullptr;
    
    switch (format)
    {
        case ExportFormat::WAV:
            audioFormat = formatManager_->findFormatForFileExtension(".wav");
            break;
            
        case ExportFormat::MP3:
#if JUCE_USE_LAME_AUDIO_FORMAT
            audioFormat = formatManager_->findFormatForFileExtension(".mp3");
#endif
            break;
            
        case ExportFormat::FLAC:
#if JUCE_USE_FLAC
            audioFormat = formatManager_->findFormatForFileExtension(".flac");
#endif
            break;
            
        case ExportFormat::OGG:
#if JUCE_USE_OGGVORBIS
            audioFormat = formatManager_->findFormatForFileExtension(".ogg");
#endif
            break;
    }
    
    if (!audioFormat)
        return nullptr;
    
    // Setup format-specific options
    juce::StringPairArray formatOptions;
    
    if (format == ExportFormat::MP3)
    {
        // MP3 quality settings
        int bitrate = static_cast<int>(settings.quality * 320); // 0-320 kbps
        bitrate = juce::jmax(64, bitrate);
        formatOptions.set("bitrate", juce::String(bitrate));
    }
    else if (format == ExportFormat::OGG)
    {
        // OGG quality settings (0.0 to 1.0)
        formatOptions.set("quality", juce::String(settings.quality));
    }
    
    return std::unique_ptr<juce::AudioFormatWriter>(
        audioFormat->createWriterFor(outputStream,
                                   settings.sampleRate,
                                   settings.numChannels,
                                   settings.bitDepth,
                                   formatOptions,
                                   0));
}

void ExportEngine::updateProgress(double progress)
{
    exportProgress_.store(juce::jlimit(0.0, 1.0, progress));
    
    if (onProgressUpdate)
        onProgressUpdate(exportProgress_.load());
}

void ExportEngine::updateOperation(const std::string& operation)
{
    {
        std::lock_guard<std::mutex> lock(operationMutex_);
        currentOperation_ = operation;
    }
    
    if (onOperationUpdate)
        onOperationUpdate(operation);
}

void ExportEngine::applyFades(juce::AudioBuffer<float>& buffer, 
                             const ExportSettings& settings, 
                             double currentTimeSeconds)
{
    int numSamples = buffer.getNumSamples();
    double sampleRate = settings.sampleRate;
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        double sampleTime = currentTimeSeconds + (sample / sampleRate);
        float gain = 1.0f;
        
        // Fade in
        if (settings.fadeIn)
        {
            double fadeInEnd = settings.startTimeSeconds + settings.fadeInDuration;
            if (sampleTime <= fadeInEnd)
            {
                double fadeProgress = (sampleTime - settings.startTimeSeconds) / settings.fadeInDuration;
                gain *= static_cast<float>(juce::jlimit(0.0, 1.0, fadeProgress));
            }
        }
        
        // Fade out
        if (settings.fadeOut)
        {
            double fadeOutStart = settings.endTimeSeconds - settings.fadeOutDuration;
            if (sampleTime >= fadeOutStart)
            {
                double fadeProgress = (settings.endTimeSeconds - sampleTime) / settings.fadeOutDuration;
                gain *= static_cast<float>(juce::jlimit(0.0, 1.0, fadeProgress));
            }
        }
        
        // Apply gain to all channels
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            buffer.setSample(channel, sample, buffer.getSample(channel, sample) * gain);
        }
    }
}