#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <atomic>
#include <mutex>

#include "AnalysisWorker.h"

class WaveformView : public juce::Component, public juce::Timer
{
public:
    WaveformView();
    ~WaveformView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    // Data input
    void setWaveformData(const std::vector<float>& audioData, double sampleRate, int channels);
    void clearWaveformData();
    
    // Analysis display
    void setAnalysisResults(const AnalysisResult& results);
    
    // Playback position
    void setPlaybackPosition(double positionSeconds);
    void setTotalDuration(double durationSeconds);
    
    // Loop points
    void setLoopPoints(double startSeconds, double endSeconds);
    void setLoopEnabled(bool enabled);
    
    // View control
    void setZoom(double zoomFactor);
    void setViewStart(double startSeconds);
    
    // Callbacks
    std::function<void(double)> onSeekRequested;
    std::function<void(double, double)> onLoopPointsChanged;

    // Timer callback for updates
    void timerCallback() override;

private:
    // Waveform data
    std::vector<float> waveformData;
    double sampleRate = 44100.0;
    int numChannels = 2;
    double totalDuration = 0.0;
    
    // Analysis data (thread-safe)
    mutable std::mutex analysisMutex;
    AnalysisResult currentAnalysis;
    
    // Playback state
    std::atomic<double> playbackPosition{0.0};
    
    // Loop state
    std::atomic<double> loopStart{0.0};
    std::atomic<double> loopEnd{0.0};
    std::atomic<bool> loopEnabled{false};
    
    // View state
    double zoomFactor = 1.0;
    double viewStartSeconds = 0.0;
    double viewEndSeconds = 0.0;
    
    // UI state
    bool isDraggingLoopStart = false;
    bool isDraggingLoopEnd = false;
    int mouseDownX = 0;
    
    // Cached drawing data
    std::vector<float> displayPeaks;
    int lastWidth = 0;
    
    // Drawing methods
    void generateDisplayPeaks();
    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> area);
    void drawBeatGrid(juce::Graphics& g, juce::Rectangle<int> area);
    void drawLoopPoints(juce::Graphics& g, juce::Rectangle<int> area);
    void drawPlaybackPosition(juce::Graphics& g, juce::Rectangle<int> area);
    
    // Utility methods
    double pixelToTime(int pixel, juce::Rectangle<int> area) const;
    int timeToPixel(double time, juce::Rectangle<int> area) const;
    void updateViewRange();
    juce::Colour getWaveformColour() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformView)
};