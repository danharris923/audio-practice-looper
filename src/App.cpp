#include "App.h"

void AudioPracticeLooperApp::initialise(const juce::String& commandLine)
{
    juce::ignoreUnused(commandLine);

    mainWindow = std::make_unique<MainWindow>(getApplicationName());
    mainWindow->setVisible(true);
}

void AudioPracticeLooperApp::shutdown()
{
    mainWindow = nullptr;
}

void AudioPracticeLooperApp::systemRequestedQuit()
{
    quit();
}

void AudioPracticeLooperApp::anotherInstanceStarted(const juce::String& commandLine)
{
    juce::ignoreUnused(commandLine);
}