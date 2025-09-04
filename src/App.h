#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "MainWindow.h"

class AudioPracticeLooperApp : public juce::JUCEApplication
{
public:
    AudioPracticeLooperApp() = default;

    const juce::String getApplicationName() override
    {
        return ProjectInfo::projectName;
    }

    const juce::String getApplicationVersion() override
    {
        return ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed() override
    {
        return true;
    }

    void initialise(const juce::String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;
    void anotherInstanceStarted(const juce::String& commandLine) override;

private:
    std::unique_ptr<MainWindow> mainWindow;
};

namespace ProjectInfo
{
    const char* const projectName = "Audio Practice Looper";
    const char* const companyName = "Audio Practice Looper Project";
    const char* const versionString = "1.0.0";
    const int versionNumber = 0x10000;
}