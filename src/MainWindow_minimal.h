#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SimpleAudioEngine.h"

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

class MainComponent : public juce::Component, private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Header
    juce::Label titleLabel;
    
    // File loading section
    juce::GroupComponent fileGroup;
    juce::TextButton loadFileButton;
    juce::Label fileStatusLabel;
    
    // Transport controls (improved)
    juce::GroupComponent transportGroup;
    juce::TextButton playPauseButton;     // Combined play/pause with proper icons
    juce::TextButton stopButton;
    juce::Label positionLabel;
    
    // Settings menu
    juce::TextButton settingsMenuButton;  // Hamburger menu
    
    // Tempo/Pitch controls
    juce::GroupComponent effectsGroup;
    juce::Label tempoLabel;
    juce::Slider tempoSlider;
    juce::Label tempoValueLabel;
    juce::Label pitchLabel;
    juce::Slider pitchSlider;
    juce::Label pitchValueLabel;
    
    // BPM and Grid controls
    juce::Label bpmLabel;
    juce::Slider bpmSlider;
    juce::Label bpmValueLabel;
    juce::TextButton detectBPMButton;
    
    // Advanced Loop controls (Song Master style)
    juce::GroupComponent loopGroup;
    juce::TextButton abLoopButton;        // Single A/B toggle button
    juce::TextButton clearLoopButton;     // Clear loop and continue
    
    // Fine-tuning jog buttons
    juce::TextButton jogALeftButton;      // A point -0.005s
    juce::TextButton jogARightButton;     // A point +0.005s  
    juce::TextButton jogBLeftButton;      // B point -0.005s
    juce::TextButton jogBRightButton;     // B point +0.005s
    
    // Loop length controls
    juce::TextButton doubleLoopButton;    // 2x loop length
    juce::TextButton halveLoopButton;     // 0.5x loop length
    
    // Bar movement controls
    juce::TextButton moveLoopBackButton;  // Move loop backward
    juce::TextButton moveLoopForwardButton; // Move loop forward
    
    juce::Label loopInfoLabel;
    
    // Recording controls
    juce::GroupComponent recordingGroup;
    juce::TextButton recordButton;
    juce::TextButton saveRecordingButton;
    juce::ToggleButton inputMonitorButton;
    juce::Label recordingStatusLabel;
    
    // Waveform display
    juce::Component waveformArea;
    
    // Status
    juce::Label statusLabel;
    
    // Audio engine
    std::unique_ptr<SimpleAudioEngine> audioEngine;
    
    // Timer callback
    void timerCallback() override;
    
    // Helper methods
    void updateTempoLabel();
    void updatePitchLabel();
    void updatePositionLabel();
    void updateUIState();
    void updateLoopInfo();
    void drawWaveformPlaceholder(juce::Graphics& g, juce::Rectangle<int> bounds);
    void loadAudioFile();
    void saveRecording();
    
    // Advanced loop control helpers
    void updateABButtonState();
    void showSettingsMenu();
    void styleButton(juce::Button& button, juce::Colour color = juce::Colour(0xff404040));
    
    // State tracking
    enum LoopState { NoLoop, HasA, HasAB };
    LoopState currentLoopState = NoLoop;
    bool isCurrentlyPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};