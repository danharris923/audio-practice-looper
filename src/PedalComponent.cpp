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
    
    // Digital display setup
    digitalDisplay.setText("READY", juce::dontSendNotification);
    digitalDisplay.setJustificationType(juce::Justification::centred);
    digitalDisplay.setColour(juce::Label::textColourId, juce::Colour(0xff00ff00)); // Bright green like ytLooper
    digitalDisplay.setColour(juce::Label::backgroundColourId, juce::Colour(0xff000000));
    digitalDisplay.setFont(juce::Font("Courier New", 16.0f, juce::Font::bold)); // Monospace for digital look
    addAndMakeVisible(digitalDisplay);
    
    displayArea.setPaintingIsUnclipped(true);
    addAndMakeVisible(displayArea);
    
    // Start timer for LED updates and display refresh
    startTimer(100);
}

PedalComponent::~PedalComponent()
{
    stopTimer();
}

void PedalComponent::paint(juce::Graphics& g)
{
    paintMetalPedalBackground(g);
    
    // Boss logo and branding (top section)
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 20.0f, juce::Font::bold));
    g.drawText("BOSS", juce::Rectangle<int>(20, 15, 60, 25), juce::Justification::left);
    
    // Loop Station text (centered)
    g.setFont(juce::Font("Arial", 14.0f, juce::Font::plain));
    g.drawText("Loop Station", getLocalBounds().withY(18).withHeight(20), juce::Justification::centred);
    
    // Model RC-1 (top right)
    g.setFont(juce::Font("Arial", 16.0f, juce::Font::bold));
    g.drawText("RC-1", juce::Rectangle<int>(getWidth() - 70, 15, 50, 25), juce::Justification::right);
    
    // Paint digital display area (like ytLooper extension)
    auto displayBounds = displayArea.getBounds();
    if (!displayBounds.isEmpty())
    {
        paintDigitalDisplay(g, displayBounds);
    }
    
    // Paint single LED indicator (Boss RC-1 has one LED)
    int ledSize = 8;
    int ledX = displayBounds.getRight() + 20;
    int ledY = displayBounds.getY() + displayBounds.getHeight() / 2 - ledSize / 2;
    juce::Rectangle<int> ledBounds(ledX, ledY, ledSize, ledSize);
    
    // LED color based on state
    juce::Colour ledColor = juce::Colours::darkgrey;
    bool ledOn = false;
    
    if (recordLED)
    {
        ledColor = juce::Colours::red;
        ledOn = true;
    }
    else if (playLED)
    {
        ledColor = juce::Colours::green;
        ledOn = true;
    }
    
    paintLED(g, ledBounds, ledColor, ledOn);
    
    // Input/Output labels (like on real RC-1)
    g.setColour(juce::Colour(0xff333333));
    g.setFont(juce::Font("Arial", 9.0f, juce::Font::plain));
    g.drawText("INPUT", juce::Rectangle<int>(30, getHeight() - 25, 40, 15), juce::Justification::left);
    g.drawText("OUTPUT", juce::Rectangle<int>(getWidth() - 70, getHeight() - 25, 50, 15), juce::Justification::right);
}

void PedalComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Top area for branding
    bounds.removeFromTop(50);
    
    // Digital display area (like ytLooper extension)
    auto displayBounds = bounds.removeFromTop(60);
    displayArea.setBounds(displayBounds.reduced(40, 10));
    digitalDisplay.setBounds(displayArea.getBounds().reduced(5));
    
    // Knobs area (below display)
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
    updateDigitalDisplay();
    repaint(); // Repaint for LED animation and display updates
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

void PedalComponent::updateDigitalDisplay()
{
    if (!audioEngine)
    {
        digitalDisplay.setText("NO ENGINE", juce::dontSendNotification);
        return;
    }
    
    auto state = audioEngine->getLoopRecordState();
    juce::String displayText;
    
    switch (state)
    {
        case SimpleAudioEngine::LoopRecordState::Idle:
            if (audioEngine->isFileLoaded())
            {
                // Show current position like ytLooper
                double position = audioEngine->getPosition();
                double duration = audioEngine->getDuration();
                int mins = (int)(position / 60.0);
                int secs = (int)(position - mins * 60.0);
                int totalMins = (int)(duration / 60.0);
                int totalSecs = (int)(duration - totalMins * 60.0);
                displayText = juce::String::formatted("%02d:%02d / %02d:%02d", mins, secs, totalMins, totalSecs);
            }
            else
            {
                displayText = "LOAD FILE";
            }
            break;
            
        case SimpleAudioEngine::LoopRecordState::Recording:
            displayText = "RECORDING...";
            break;
            
        case SimpleAudioEngine::LoopRecordState::Looping:
        {
            // Show loop info like ytLooper
            double loopStart = audioEngine->getLoopStart();
            double loopEnd = audioEngine->getLoopEnd();
            double loopLength = loopEnd - loopStart;
            int mins = (int)(loopLength / 60.0);
            int secs = (int)(loopLength - mins * 60.0);
            displayText = juce::String::formatted("LOOP %02d:%02d", mins, secs);
            break;
        }
    }
    
    digitalDisplay.setText(displayText, juce::dontSendNotification);
}

void PedalComponent::paintMetalPedalBackground(juce::Graphics& g)
{
    // Boss RC-1 style brushed metal background (more realistic)
    juce::Rectangle<float> bounds = getLocalBounds().toFloat();
    
    // Silver/aluminum metal body like real RC-1
    juce::Colour metalBase(0xffcccccc);      // Light aluminum
    juce::Colour metalHighlight(0xffeeeeee); // Bright highlight
    juce::Colour metalShadow(0xff999999);    // Darker shadow
    
    // Main brushed metal gradient
    juce::ColourGradient metalGradient(metalHighlight, bounds.getX(), bounds.getY(),
                                       metalShadow, bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(metalGradient);
    g.fillRoundedRectangle(bounds, 8.0f);
    
    // Brushed metal texture effect
    for (int i = 0; i < getHeight(); i += 2)
    {
        float alpha = 0.05f + (float)(i % 4) * 0.01f;
        g.setColour(metalHighlight.withAlpha(alpha));
        g.drawHorizontalLine(i, bounds.getX(), bounds.getRight());
    }
    
    // Edge bevel effect (more subtle)
    g.setColour(metalHighlight.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 8.0f, 1.0f);
    
    g.setColour(metalShadow.withAlpha(0.7f));
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);
    
    // Boss-style rubber feet (black circles in corners)
    g.setColour(juce::Colour(0xff222222));
    int footSize = 12;
    g.fillEllipse(15, getHeight() - 25, footSize, footSize);
    g.fillEllipse(getWidth() - 15 - footSize, getHeight() - 25, footSize, footSize);
    
    // Screws (smaller, more realistic)
    g.setColour(juce::Colour(0xff666666));
    int screwSize = 6;
    g.fillEllipse(15, 15, screwSize, screwSize);
    g.fillEllipse(getWidth() - 15 - screwSize, 15, screwSize, screwSize);
    
    // Screw cross pattern
    g.setColour(juce::Colour(0xff444444));
    g.drawLine(18, 15, 18, 21, 1.0f);
    g.drawLine(15, 18, 21, 18, 1.0f);
    g.drawLine(getWidth() - 18, 15, getWidth() - 18, 21, 1.0f);
    g.drawLine(getWidth() - 21, 18, getWidth() - 15, 18, 1.0f);
}

void PedalComponent::paintDigitalDisplay(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Digital display background (like ytLooper extension)
    g.setColour(juce::Colour(0xff000000)); // Black background
    g.fillRoundedRectangle(bounds.toFloat(), 3.0f);
    
    // Digital display border
    g.setColour(juce::Colour(0xff333333)); // Dark gray border
    g.drawRoundedRectangle(bounds.toFloat(), 3.0f, 2.0f);
    
    // Inner shadow effect
    g.setColour(juce::Colour(0xff111111));
    g.fillRoundedRectangle(bounds.reduced(3).toFloat(), 2.0f);
    
    // Subtle green glow around the display
    g.setColour(juce::Colour(0xff00ff00).withAlpha(0.1f));
    g.fillRoundedRectangle(bounds.expanded(2).toFloat(), 5.0f);
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