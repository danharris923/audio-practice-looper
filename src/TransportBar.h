#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class TransportBar : public juce::Component
{
public:
    TransportBar();
    ~TransportBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Transport state
    void setIsPlaying(bool playing);
    void setPosition(double positionSeconds);
    void setDuration(double durationSeconds);
    
    // Tempo and pitch controls
    void setTempoPercent(int percent);
    void setPitchSemitones(int semitones);
    
    // BPM display
    void setBPM(double bpm);
    
    // Callbacks
    std::function<void()> onPlayPauseClicked;
    std::function<void()> onStopClicked;
    std::function<void(int)> onTempoChanged;  // percent (25-200)
    std::function<void(int)> onPitchChanged; // semitones (-12 to +12)

private:
    // Transport buttons
    juce::TextButton playPauseButton;
    juce::TextButton stopButton;
    
    // Tempo/pitch controls
    juce::Slider tempoSlider;
    juce::Slider pitchSlider;
    juce::Label tempoLabel;
    juce::Label pitchLabel;
    juce::Label tempoValueLabel;
    juce::Label pitchValueLabel;
    
    // Position display
    juce::Label positionLabel;
    juce::Label durationLabel;
    juce::Label bpmLabel;
    
    // State
    bool isPlaying = false;
    double currentPosition = 0.0;
    double totalDuration = 0.0;
    int tempoPercent = 100;
    int pitchSemitones = 0;
    double currentBPM = 0.0;
    
    // Helper methods
    void updatePlayPauseButton();
    void updateTempoLabel();
    void updatePitchLabel();
    void updatePositionLabels();
    juce::String formatTime(double seconds) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};