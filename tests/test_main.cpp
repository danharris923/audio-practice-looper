#define JUCE_UNIT_TESTS 1
#include <juce_core/juce_core.h>

int main()
{
    // Initialize JUCE for testing
    juce::UnitTestRunner runner;
    
    // Add test classes
    runner.runAllTests();
    
    // Print results
    auto numTests = runner.getNumTests();
    auto numPasses = runner.getNumPasses();
    auto numFailures = runner.getNumFailures();
    
    std::cout << "\n=== Test Results ===" << std::endl;
    std::cout << "Total Tests: " << numTests << std::endl;
    std::cout << "Passed: " << numPasses << std::endl;
    std::cout << "Failed: " << numFailures << std::endl;
    
    if (numFailures > 0)
    {
        std::cout << "\n❌ Some tests failed!" << std::endl;
        return 1;
    }
    else
    {
        std::cout << "\n✅ All tests passed!" << std::endl;
        return 0;
    }
}