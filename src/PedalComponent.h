#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SimpleAudioEngine.h"

// Boss RC-style loop pedal component
class PedalComponent : public juce::Component, private juce::Timer
{
public:
    PedalComponent(SimpleAudioEngine* engine);
    ~PedalComponent() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
private:
    SimpleAudioEngine* audioEngine;
    
    // Boss RC-style controls
    juce::TextButton mainStompButton;  // Big record/play/stop button
    juce::Label stompLabel;
    
    // Rotary knobs (Boss style)
    juce::Slider levelKnob;
    juce::Label levelLabel;
    
    juce::Slider loopFxKnob;  // Loop overlap/compensation
    juce::Label loopFxLabel;
    
    juce::Slider tempoKnob;
    juce::Label tempoLabel;
    
    // LED indicators
    bool recordLED = false;
    bool playLED = false;
    juce::Colour ledColor = juce::Colours::darkgrey;
    
    // Smaller control buttons (like Boss pedals)
    juce::TextButton tapTempoButton;
    juce::TextButton undoRedoButton;
    juce::TextButton stopButton;
    
    // Timer callback for LED animation
    void timerCallback() override;
    void updateLEDState();
    void paintMetalPedalBackground(juce::Graphics& g);
    void paintLED(juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour color, bool isOn);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PedalComponent)
};