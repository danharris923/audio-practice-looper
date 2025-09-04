#include <juce_core/juce_core.h>
#include "AudioEngine.h"

class AudioEngineTests : public juce::UnitTest
{
public:
    AudioEngineTests() : juce::UnitTest("AudioEngine Tests") {}

    void runTest() override
    {
        beginTest("AudioEngine Initialization");
        {
            AudioEngine engine;
            engine.initialize();
            
            expect(!engine.isPlaying(), "Engine should not be playing initially");
            expect(engine.getTempoRatio() == 1.0f, "Initial tempo ratio should be 1.0");
            expect(engine.getPitchSemitones() == 0, "Initial pitch should be 0 semitones");
            expect(engine.getLoopInSeconds() == 0.0, "Initial loop in should be 0.0");
            expect(engine.getLoopOutSeconds() == 0.0, "Initial loop out should be 0.0");
            expect(!engine.getLoopEnabled(), "Loop should be disabled initially");
            
            engine.shutdown();
        }

        beginTest("AudioEngine Parameter Setting");
        {
            AudioEngine engine;
            engine.initialize();
            
            // Test tempo ratio
            engine.setTempoRatio(1.5f);
            expectWithinAbsoluteError(engine.getTempoRatio(), 1.5f, 0.001f, 
                                    "Tempo ratio should be set correctly");
            
            // Test tempo ratio clamping
            engine.setTempoRatio(5.0f); // Above max
            expect(engine.getTempoRatio() <= 4.0f, "Tempo ratio should be clamped to maximum");
            
            engine.setTempoRatio(0.1f); // Below min  
            expect(engine.getTempoRatio() >= 0.25f, "Tempo ratio should be clamped to minimum");
            
            // Test pitch setting
            engine.setPitchSemitones(5);
            expect(engine.getPitchSemitones() == 5, "Pitch should be set correctly");
            
            // Test pitch clamping
            engine.setPitchSemitones(30); // Above max
            expect(engine.getPitchSemitones() <= 24, "Pitch should be clamped to maximum");
            
            engine.setPitchSemitones(-30); // Below min
            expect(engine.getPitchSemitones() >= -24, "Pitch should be clamped to minimum");
            
            engine.shutdown();
        }

        beginTest("AudioEngine Loop Control");
        {
            AudioEngine engine;
            engine.initialize();
            
            // Set loop points
            engine.setLoopInSeconds(10.0);
            engine.setLoopOutSeconds(20.0);
            
            expectWithinAbsoluteError(engine.getLoopInSeconds(), 10.0, 0.001, 
                                    "Loop in should be set correctly");
            expectWithinAbsoluteError(engine.getLoopOutSeconds(), 20.0, 0.001, 
                                    "Loop out should be set correctly");
            
            // Enable loop
            engine.setLoopEnabled(true);
            expect(engine.getLoopEnabled(), "Loop should be enabled");
            
            // Disable loop
            engine.setLoopEnabled(false);
            expect(!engine.getLoopEnabled(), "Loop should be disabled");
            
            engine.shutdown();
        }

        beginTest("AudioEngine Playback Control");
        {
            AudioEngine engine;
            engine.initialize();
            
            // Test play/pause/stop
            engine.play();
            expect(engine.isPlaying(), "Engine should be playing after play()");
            
            engine.pause();
            expect(!engine.isPlaying(), "Engine should not be playing after pause()");
            
            engine.play();
            expect(engine.isPlaying(), "Engine should be playing after play() again");
            
            engine.stop();
            expect(!engine.isPlaying(), "Engine should not be playing after stop()");
            
            engine.shutdown();
        }

        beginTest("AudioEngine Analysis Integration");
        {
            AudioEngine engine;
            engine.initialize();
            
            // Test analysis settings
            engine.setAnalysisEnabled(true);
            engine.setAnalysisEnabled(false);
            
            // Get analysis results (should be empty/invalid initially)
            auto results = engine.getAnalysisResults();
            expect(!results.isValid, "Analysis results should be invalid without audio");
            expect(results.bpm == 0.0, "BPM should be 0 without analysis data");
            expect(results.beats.empty(), "Beats array should be empty without analysis data");
            
            engine.shutdown();
        }
    }
};

static AudioEngineTests audioEngineTests;