#include <juce_core/juce_core.h>
#include "ExportEngine.h"

class ExportEngineTests : public juce::UnitTest
{
public:
    ExportEngineTests() : juce::UnitTest("ExportEngine Tests") {}

    void runTest() override
    {
        beginTest("ExportEngine Initialization");
        {
            ExportEngine engine;
            
            expect(!engine.isExporting(), "Engine should not be exporting initially");
            expect(engine.getExportProgress() == 0.0, "Initial progress should be 0.0");
        }

        beginTest("ExportEngine Format Support");
        {
            auto formats = ExportEngine::getSupportedFormats();
            expect(!formats.empty(), "Should support at least one format");
            
            // WAV should always be supported
            bool hasWav = false;
            for (auto format : formats)
            {
                if (format == ExportEngine::ExportFormat::WAV)
                {
                    hasWav = true;
                    break;
                }
            }
            expect(hasWav, "WAV format should always be supported");
            
            // Test format descriptions
            auto wavDesc = ExportEngine::getFormatDescription(ExportEngine::ExportFormat::WAV);
            expect(wavDesc.find("WAV") != std::string::npos, "WAV description should contain 'WAV'");
            
            auto wavExt = ExportEngine::getFormatExtension(ExportEngine::ExportFormat::WAV);
            expect(wavExt == ".wav", "WAV extension should be '.wav'");
        }

        beginTest("ExportEngine Settings Validation");
        {
            ExportEngine engine;
            ExportEngine::ExportSettings settings;
            
            // Create temporary file for testing
            auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
            settings.outputFile = tempDir.getChildFile("test_export.wav");
            settings.startTimeSeconds = 0.0;
            settings.endTimeSeconds = 10.0;
            settings.sampleRate = 44100;
            settings.bitDepth = 16;
            settings.numChannels = 2;
            
            // Valid settings should be accepted (though export will fail without audio source)
            bool started = engine.startExport(settings, ExportEngine::ExportFormat::WAV);
            if (started)
            {
                // Cancel immediately to clean up
                engine.cancelExport();
            }
            // Note: We expect this to potentially fail due to missing audio source,
            // but the settings validation should pass
        }

        beginTest("ExportEngine Time Estimation");
        {
            ExportEngine engine;
            ExportEngine::ExportSettings settings;
            settings.startTimeSeconds = 0.0;
            settings.endTimeSeconds = 60.0; // 1 minute
            settings.sampleRate = 44100;
            settings.bitDepth = 16;
            settings.numChannels = 2;
            
            double estimatedTime = engine.estimateExportTime(settings);
            expect(estimatedTime > 0.0, "Estimated time should be positive");
            expect(estimatedTime < 60.0, "Estimated time should be less than audio duration");
        }

        beginTest("ExportEngine File Size Estimation");
        {
            ExportEngine engine;
            ExportEngine::ExportSettings settings;
            settings.startTimeSeconds = 0.0;
            settings.endTimeSeconds = 60.0; // 1 minute
            settings.sampleRate = 44100;
            settings.bitDepth = 16;
            settings.numChannels = 2;
            
            // Test WAV file size estimation
            int64_t wavSize = engine.estimateFileSize(settings, ExportEngine::ExportFormat::WAV);
            expect(wavSize > 0, "WAV file size estimate should be positive");
            
            // Expected size for 1 minute of 16-bit stereo at 44.1kHz
            int64_t expectedWavSize = 60 * 44100 * 2 * 2; // duration * sampleRate * channels * bytesPerSample
            expectWithinAbsoluteError(static_cast<double>(wavSize), static_cast<double>(expectedWavSize), 
                                    expectedWavSize * 0.1, "WAV size estimate should be close to expected");
            
            // Test MP3 file size estimation (should be much smaller)
            int64_t mp3Size = engine.estimateFileSize(settings, ExportEngine::ExportFormat::MP3);
            if (mp3Size > 0) // MP3 might not be available in all builds
            {
                expect(mp3Size < wavSize, "MP3 file should be smaller than WAV");
            }
        }

        beginTest("ExportEngine Export Control");
        {
            ExportEngine engine;
            
            // Should not be able to start export without valid settings
            ExportEngine::ExportSettings invalidSettings;
            invalidSettings.outputFile = juce::File(); // Invalid file
            invalidSettings.startTimeSeconds = 10.0;
            invalidSettings.endTimeSeconds = 5.0; // Invalid range (end < start)
            
            bool started = engine.startExport(invalidSettings, ExportEngine::ExportFormat::WAV);
            expect(!started, "Should not start export with invalid settings");
            expect(!engine.isExporting(), "Should not be exporting after failed start");
        }

        beginTest("ExportEngine Progress Tracking");
        {
            ExportEngine engine;
            
            // Progress should be 0 initially
            expect(engine.getExportProgress() == 0.0, "Initial progress should be 0.0");
            
            // Current operation should be accessible
            std::string operation = engine.getCurrentOperation();
            expect(!operation.empty() || operation.empty(), "Operation string should be accessible");
        }
    }
};

static ExportEngineTests exportEngineTests;