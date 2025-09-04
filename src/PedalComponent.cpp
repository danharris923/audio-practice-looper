#include "PedalComponent.h"

PedalComponent::PedalComponent(SimpleAudioEngine* engine) : audioEngine(engine)
{
    // Main stomp button - Boss RC style
    mainStompButton.setButtonText("");
    mainStompButton.onClick = [this]()
    {
        if (audioEngine)
        {
            audioEngine->toggleLoopRecord();
            updateLEDState();
        }
    };
    addAndMakeVisible(mainStompButton);
    
    stompLabel.setText("REC/PLAY/STOP", juce::dontSendNotification);
    stompLabel.setJustificationType(juce::Justification::centred);
    stompLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(stompLabel);
    
    // Level knob (Boss style)
    levelKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    levelKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    levelKnob.setRange(0.0, 100.0, 1.0);
    levelKnob.setValue(75.0);
    levelKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff8b0000));
    levelKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff333333));
    addAndMakeVisible(levelKnob);
    
    levelLabel.setText("LEVEL", juce::dontSendNotification);
    levelLabel.setJustificationType(juce::Justification::centred);
    levelLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    levelLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    addAndMakeVisible(levelLabel);
    
    // Loop FX knob (overlap/compensation)
    loopFxKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    loopFxKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    loopFxKnob.setRange(0.0, 300.0, 10.0);
    loopFxKnob.setValue(100.0);
    loopFxKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff8b0000));
    loopFxKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff333333));
    loopFxKnob.onValueChange = [this]()
    {
        if (audioEngine)
        {
            audioEngine->setLoopOverlapMs((int)loopFxKnob.getValue());
        }
    };
    addAndMakeVisible(loopFxKnob);
    
    loopFxLabel.setText("LOOP FX", juce::dontSendNotification);
    loopFxLabel.setJustificationType(juce::Justification::centred);
    loopFxLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    loopFxLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    addAndMakeVisible(loopFxLabel);
    
    // Tempo knob
    tempoKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    tempoKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    tempoKnob.setRange(60.0, 200.0, 1.0);
    tempoKnob.setValue(120.0);
    tempoKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff8b0000));
    tempoKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff333333));
    tempoKnob.onValueChange = [this]()
    {
        if (audioEngine)
        {
            audioEngine->setBPM(tempoKnob.getValue());
        }
    };
    addAndMakeVisible(tempoKnob);
    
    tempoLabel.setText("TEMPO", juce::dontSendNotification);
    tempoLabel.setJustificationType(juce::Justification::centred);
    tempoLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    tempoLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    addAndMakeVisible(tempoLabel);
    
    // Small control buttons
    tapTempoButton.setButtonText("TAP");
    tapTempoButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff555555));
    tapTempoButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(tapTempoButton);
    
    undoRedoButton.setButtonText("UNDO");
    undoRedoButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff555555));
    undoRedoButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(undoRedoButton);
    
    stopButton.setButtonText("STOP");
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff555555));
    stopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    stopButton.onClick = [this]()
    {
        if (audioEngine)
        {
            audioEngine->stop();
            audioEngine->clearLoop();
            updateLEDState();
        }
    };
    addAndMakeVisible(stopButton);
    
    // Start timer for LED updates
    startTimer(100);
}

PedalComponent::~PedalComponent()
{
    stopTimer();
}

void PedalComponent::paint(juce::Graphics& g)
{
    paintMetalPedalBackground(g);
    
    // Paint LED indicators
    int ledY = getHeight() * 0.15;
    int ledSize = 12;
    
    // Record LED (left)
    juce::Rectangle<int> recLedBounds(getWidth() * 0.3 - ledSize/2, ledY, ledSize, ledSize);
    paintLED(g, recLedBounds, juce::Colours::red, recordLED);
    
    // Play LED (right) 
    juce::Rectangle<int> playLedBounds(getWidth() * 0.7 - ledSize/2, ledY, ledSize, ledSize);
    paintLED(g, playLedBounds, juce::Colours::green, playLED);
    
    // Boss logo style text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("LOOP STATION", getLocalBounds().removeFromTop(30), juce::Justification::centred);
    
    // Model number
    g.setFont(juce::Font(11.0f));
    g.drawText("RC-1", juce::Rectangle<int>(10, 35, 50, 20), juce::Justification::left);
}

void PedalComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Top area for LEDs and title
    bounds.removeFromTop(60);
    
    // Knobs area (top row)
    auto knobArea = bounds.removeFromTop(80);
    int knobSize = 50;
    int knobSpacing = getWidth() / 4;
    
    // Level knob
    auto levelBounds = juce::Rectangle<int>(knobSpacing - knobSize/2, knobArea.getY(), knobSize, knobSize);
    levelKnob.setBounds(levelBounds);
    levelLabel.setBounds(levelBounds.withY(levelBounds.getBottom()).withHeight(15));
    
    // Loop FX knob
    auto loopFxBounds = juce::Rectangle<int>(knobSpacing * 2 - knobSize/2, knobArea.getY(), knobSize, knobSize);
    loopFxKnob.setBounds(loopFxBounds);
    loopFxLabel.setBounds(loopFxBounds.withY(loopFxBounds.getBottom()).withHeight(15));
    
    // Tempo knob
    auto tempoBounds = juce::Rectangle<int>(knobSpacing * 3 - knobSize/2, knobArea.getY(), knobSize, knobSize);
    tempoKnob.setBounds(tempoBounds);
    tempoLabel.setBounds(tempoBounds.withY(tempoBounds.getBottom()).withHeight(15));
    
    bounds.removeFromTop(20);
    
    // Main stomp button (Boss pedal style - large and centered)
    int stompSize = 80;
    auto stompBounds = juce::Rectangle<int>(getWidth()/2 - stompSize/2, 
                                            bounds.getY() + 10, 
                                            stompSize, stompSize);
    mainStompButton.setBounds(stompBounds);
    stompLabel.setBounds(stompBounds.withY(stompBounds.getBottom() + 5).withHeight(20));
    
    // Small control buttons at bottom
    bounds.removeFromTop(120);
    auto buttonArea = bounds.removeFromTop(30);
    int smallButtonWidth = 50;
    int buttonSpacing = (getWidth() - smallButtonWidth * 3) / 4;
    
    tapTempoButton.setBounds(buttonSpacing, buttonArea.getY(), smallButtonWidth, 25);
    undoRedoButton.setBounds(buttonSpacing * 2 + smallButtonWidth, buttonArea.getY(), smallButtonWidth, 25);
    stopButton.setBounds(buttonSpacing * 3 + smallButtonWidth * 2, buttonArea.getY(), smallButtonWidth, 25);
}

void PedalComponent::timerCallback()
{
    updateLEDState();
    repaint(); // Repaint for LED animation
}

void PedalComponent::updateLEDState()
{
    if (!audioEngine)
        return;
        
    auto state = audioEngine->getLoopRecordState();
    
    switch (state)
    {
        case SimpleAudioEngine::LoopRecordState::Idle:
            recordLED = false;
            playLED = false;
            mainStompButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff666666));
            break;
            
        case SimpleAudioEngine::LoopRecordState::Recording:
            recordLED = true;  // Solid red
            playLED = false;
            mainStompButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff990000));
            break;
            
        case SimpleAudioEngine::LoopRecordState::Looping:
            recordLED = false;
            playLED = true;  // Solid green
            mainStompButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff009900));
            break;
    }
}

void PedalComponent::paintMetalPedalBackground(juce::Graphics& g)
{
    // Boss pedal style metal background
    juce::Rectangle<float> bounds = getLocalBounds().toFloat();
    
    // Main metal body color
    juce::Colour metalBase(0xff8b0000);  // Dark red like Boss RC series
    juce::Colour metalHighlight(0xffaa2222);
    juce::Colour metalShadow(0xff660000);
    
    // Background gradient
    juce::ColourGradient metalGradient(metalHighlight, bounds.getX(), bounds.getY(),
                                       metalShadow, bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(metalGradient);
    g.fillRoundedRectangle(bounds, 10.0f);
    
    // Edge bevel effect
    g.setColour(metalHighlight.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 10.0f, 2.0f);
    
    g.setColour(metalShadow);
    g.drawRoundedRectangle(bounds, 10.0f, 3.0f);
    
    // Screws in corners (Boss pedal detail)
    g.setColour(juce::Colour(0xff333333));
    int screwSize = 8;
    g.fillEllipse(10, 10, screwSize, screwSize);
    g.fillEllipse(getWidth() - 10 - screwSize, 10, screwSize, screwSize);
    g.fillEllipse(10, getHeight() - 10 - screwSize, screwSize, screwSize);
    g.fillEllipse(getWidth() - 10 - screwSize, getHeight() - 10 - screwSize, screwSize, screwSize);
}

void PedalComponent::paintLED(juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour color, bool isOn)
{
    // LED bezel
    g.setColour(juce::Colour(0xff222222));
    g.fillEllipse(bounds.expanded(2).toFloat());
    
    // LED light
    if (isOn)
    {
        // Glow effect
        g.setColour(color.withAlpha(0.3f));
        g.fillEllipse(bounds.expanded(4).toFloat());
        
        // Main LED
        g.setColour(color);
        g.fillEllipse(bounds.toFloat());
        
        // Highlight
        g.setColour(color.brighter(0.5f));
        g.fillEllipse(bounds.reduced(2).toFloat());
    }
    else
    {
        // Off LED
        g.setColour(color.darker(0.8f));
        g.fillEllipse(bounds.toFloat());
    }
}