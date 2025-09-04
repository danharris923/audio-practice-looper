#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <vector>

#include "AnalysisWorker.h"

class LoopControls : public juce::Component
{
public:
    LoopControls();
    ~LoopControls() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Loop state
    void setLoopEnabled(bool enabled);
    void setLoopPoints(double startSeconds, double endSeconds);
    void setCurrentPosition(double positionSeconds);
    void setAnalysisResults(const AnalysisResult& results);
    
    // Current audio position and total duration for snap calculations
    void setTotalDuration(double durationSeconds);
    
    // Callbacks
    std::function<void(bool)> onLoopEnabledChanged;
    std::function<void(double, double)> onLoopPointsChanged;  // start, end in seconds
    std::function<void()> onLoopToCurrentPosition;

private:
    // Loop enable/disable
    juce::ToggleButton loopEnabledButton;
    
    // Quick loop operations
    juce::TextButton halfLoopButton;      // ×½ loop length
    juce::TextButton doubleLoopButton;    // ×2 loop length
    juce::TextButton shortenButton;       // -1 bar
    juce::TextButton extendButton;        // +1 bar
    
    // Snap-to-beat operations
    juce::TextButton snapStartToBeatButton;
    juce::TextButton snapEndToBeatButton;
    juce::TextButton snapBothToBeatsButton;
    
    // Loop to current position
    juce::TextButton loopHereButton;
    
    // Current state
    bool loopEnabled = false;
    double loopStartSeconds = 0.0;
    double loopEndSeconds = 0.0;
    double currentPositionSeconds = 0.0;
    double totalDurationSeconds = 0.0;
    
    // Analysis data for beat snapping
    std::mutex analysisMutex;
    AnalysisResult currentAnalysis;
    
    // Helper methods
    void updateButtonStates();
    double findNearestBeat(double timeSeconds) const;
    double getBarDuration() const;  // Duration of one bar in seconds
    std::vector<double> getBeatsInRange(double startTime, double endTime) const;
    
    // Button callbacks
    void onLoopEnabledToggled();
    void onHalfLoopClicked();
    void onDoubleLoopClicked();
    void onShortenLoopClicked();
    void onExtendLoopClicked();
    void onSnapStartToBeatClicked();
    void onSnapEndToBeatClicked();
    void onSnapBothToBeatsClicked();
    void onLoopHereClicked();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoopControls)
};