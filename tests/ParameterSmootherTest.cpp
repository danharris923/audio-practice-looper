#include <juce_core/juce_core.h>
#include "ParameterSmoother.h"

class ParameterSmootherTests : public juce::UnitTest
{
public:
    ParameterSmootherTests() : juce::UnitTest("ParameterSmoother Tests") {}

    void runTest() override
    {
        beginTest("Parameter Smoother Initialization");
        {
            ParameterSmoother<float> smoother;
            smoother.setSampleRate(44100.0);
            smoother.setSmoothingTimeMs(100.0f);
            smoother.setCurrentAndTargetValue(1.0f);
            
            expect(smoother.getCurrentValue() == 1.0f, "Initial value should be set correctly");
            expect(smoother.getTargetValue() == 1.0f, "Target value should be set correctly");
            expect(!smoother.isSmoothing(), "Should not be smoothing when current equals target");
        }

        beginTest("Parameter Smoother Transitions");
        {
            ParameterSmoother<float> smoother;
            smoother.setSampleRate(44100.0);
            smoother.setSmoothingTimeMs(100.0f);
            smoother.setCurrentAndTargetValue(0.0f);
            
            // Set new target
            smoother.setTargetValue(1.0f);
            expect(smoother.isSmoothing(), "Should be smoothing when target changes");
            
            // Process some samples
            float previousValue = smoother.getCurrentValue();
            for (int i = 0; i < 100; ++i)
            {
                float currentValue = smoother.getNextValue();
                expect(currentValue >= previousValue, "Value should be increasing");
                expect(currentValue <= 1.0f, "Value should not exceed target");
                previousValue = currentValue;
            }
        }

        beginTest("Parameter Smoother Completion");
        {
            ParameterSmoother<float> smoother;
            smoother.setSampleRate(44100.0);
            smoother.setSmoothingTimeMs(10.0f); // Very short smoothing time
            smoother.setCurrentAndTargetValue(0.0f);
            smoother.setTargetValue(1.0f);
            
            // Process enough samples to complete smoothing
            for (int i = 0; i < 1000; ++i)
            {
                smoother.getNextValue();
            }
            
            expect(!smoother.isSmoothing(), "Should not be smoothing after sufficient samples");
            expectWithinAbsoluteError(smoother.getCurrentValue(), 1.0f, 0.001f, 
                                    "Should reach target value");
        }

        beginTest("Parameter Smoother Reset");
        {
            ParameterSmoother<float> smoother;
            smoother.setSampleRate(44100.0);
            smoother.setSmoothingTimeMs(100.0f);
            smoother.setCurrentAndTargetValue(0.0f);
            smoother.setTargetValue(1.0f);
            
            // Start smoothing
            expect(smoother.isSmoothing(), "Should be smoothing");
            
            // Reset to new value
            smoother.setCurrentAndTargetValue(0.5f);
            expect(!smoother.isSmoothing(), "Should not be smoothing after reset");
            expect(smoother.getCurrentValue() == 0.5f, "Should have new current value");
            expect(smoother.getTargetValue() == 0.5f, "Should have new target value");
        }
    }
};

static ParameterSmootherTests parameterSmootherTests;