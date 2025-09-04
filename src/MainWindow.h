#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent;

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name);
    ~MainWindow() override;

    void closeButtonPressed() override;

private:
    std::unique_ptr<MainComponent> mainComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

class AudioEngine;

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Label welcomeLabel;
    juce::TextButton loadFileButton;
    juce::TextButton playPauseButton;
    juce::Slider tempoSlider;
    juce::Slider pitchSlider;
    juce::Label tempoLabel;
    juce::Label pitchLabel;

    // Audio engine
    std::unique_ptr<AudioEngine> audioEngine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};