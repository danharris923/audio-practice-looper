#include "TransportBar.h"

TransportBar::TransportBar()
{
    // Setup transport buttons
    playPauseButton.setButtonText("Play");
    playPauseButton.onClick = [this]() 
    {
        if (onPlayPauseClicked) 
            onPlayPauseClicked(); 
    };
    addAndMakeVisible(playPauseButton);
    
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this]() 
    { 
        if (onStopClicked) 
            onStopClicked(); 
    };
    addAndMakeVisible(stopButton);
    
    // Setup tempo controls
    tempoLabel.setText("Tempo:", juce::dontSendNotification);
    tempoLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(tempoLabel);
    
    tempoSlider.setRange(25.0, 200.0, 1.0);
    tempoSlider.setValue(100.0);
    tempoSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0);
    tempoSlider.onValueChange = [this]()
    {
        tempoPercent = static_cast<int>(tempoSlider.getValue());
        updateTempoLabel();
        if (onTempoChanged)
            onTempoChanged(tempoPercent);
    };
    addAndMakeVisible(tempoSlider);
    
    tempoValueLabel.setText("100%", juce::dontSendNotification);
    tempoValueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(tempoValueLabel);
    
    // Setup pitch controls
    pitchLabel.setText("Pitch:", juce::dontSendNotification);
    pitchLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(pitchLabel);
    
    pitchSlider.setRange(-12.0, 12.0, 1.0);
    pitchSlider.setValue(0.0);
    pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0);
    pitchSlider.onValueChange = [this]()
    {
        pitchSemitones = static_cast<int>(pitchSlider.getValue());
        updatePitchLabel();
        if (onPitchChanged)
            onPitchChanged(pitchSemitones);
    };
    addAndMakeVisible(pitchSlider);
    
    pitchValueLabel.setText("0", juce::dontSendNotification);
    pitchValueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(pitchValueLabel);
    
    // Setup position/time displays
    positionLabel.setText("00:00.0", juce::dontSendNotification);
    positionLabel.setJustificationType(juce::Justification::centred);
    positionLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(positionLabel);
    
    durationLabel.setText("/ 00:00.0", juce::dontSendNotification);
    durationLabel.setJustificationType(juce::Justification::centredLeft);
    durationLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(durationLabel);
    
    bpmLabel.setText("BPM: --", juce::dontSendNotification);
    bpmLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bpmLabel);
}

TransportBar::~TransportBar() = default;

void TransportBar::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 1);
    
    // Section dividers
    auto bounds = getLocalBounds().reduced(4);
    int sectionWidth = bounds.getWidth() / 4;
    
    g.setColour(juce::Colours::grey.withAlpha(0.3f));
    for (int i = 1; i < 4; ++i)
    {
        int x = bounds.getX() + i * sectionWidth;
        g.drawVerticalLine(x, bounds.getY(), bounds.getBottom());
    }
}

void TransportBar::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    int sectionWidth = bounds.getWidth() / 4;
    
    // Section 1: Transport buttons
    auto transportSection = bounds.removeFromLeft(sectionWidth).reduced(4);
    auto buttonRow = transportSection.removeFromTop(30);
    
    int buttonWidth = (buttonRow.getWidth() - 8) / 2;
    playPauseButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(8);
    stopButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    
    // Section 2: Tempo controls
    auto tempoSection = bounds.removeFromLeft(sectionWidth).reduced(4);
    tempoLabel.setBounds(tempoSection.removeFromTop(20));
    auto tempoSliderRow = tempoSection.removeFromTop(25);
    tempoSlider.setBounds(tempoSliderRow.removeFromLeft(tempoSliderRow.getWidth() - 50));
    tempoValueLabel.setBounds(tempoSliderRow.reduced(2));
    
    // Section 3: Pitch controls  
    auto pitchSection = bounds.removeFromLeft(sectionWidth).reduced(4);
    pitchLabel.setBounds(pitchSection.removeFromTop(20));
    auto pitchSliderRow = pitchSection.removeFromTop(25);
    pitchSlider.setBounds(pitchSliderRow.removeFromLeft(pitchSliderRow.getWidth() - 50));
    pitchValueLabel.setBounds(pitchSliderRow.reduced(2));
    
    // Section 4: Position/BPM display
    auto displaySection = bounds.reduced(4);
    bpmLabel.setBounds(displaySection.removeFromBottom(20));
    
    auto timeRow = displaySection.removeFromTop(25);
    int positionWidth = timeRow.getWidth() * 0.4f;
    positionLabel.setBounds(timeRow.removeFromLeft(positionWidth));
    durationLabel.setBounds(timeRow);
}

void TransportBar::setIsPlaying(bool playing)
{
    isPlaying = playing;
    updatePlayPauseButton();
}

void TransportBar::setPosition(double positionSeconds)
{
    currentPosition = positionSeconds;
    updatePositionLabels();
}

void TransportBar::setDuration(double durationSeconds)
{
    totalDuration = durationSeconds;
    updatePositionLabels();
}

void TransportBar::setTempoPercent(int percent)
{
    tempoPercent = juce::jlimit(25, 200, percent);
    tempoSlider.setValue(tempoPercent, juce::dontSendNotification);
    updateTempoLabel();
}

void TransportBar::setPitchSemitones(int semitones)
{
    pitchSemitones = juce::jlimit(-12, 12, semitones);
    pitchSlider.setValue(pitchSemitones, juce::dontSendNotification);
    updatePitchLabel();
}

void TransportBar::setBPM(double bpm)
{
    currentBPM = bpm;
    
    if (bpm > 0.0)
    {
        bpmLabel.setText("BPM: " + juce::String(bpm, 1), juce::dontSendNotification);
    }
    else
    {
        bpmLabel.setText("BPM: --", juce::dontSendNotification);
    }
}

void TransportBar::updatePlayPauseButton()
{
    playPauseButton.setButtonText(isPlaying ? "Pause" : "Play");
    
    // Color coding
    if (isPlaying)
    {
        playPauseButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    }
    else
    {
        playPauseButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    }
}

void TransportBar::updateTempoLabel()
{
    tempoValueLabel.setText(juce::String(tempoPercent) + "%", juce::dontSendNotification);
    
    // Color coding based on tempo
    if (tempoPercent == 100)
    {
        tempoValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    }
    else if (tempoPercent < 100)
    {
        tempoValueLabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);
    }
    else
    {
        tempoValueLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    }
}

void TransportBar::updatePitchLabel()
{
    juce::String pitchText;
    if (pitchSemitones >= 0)
    {
        pitchText = "+" + juce::String(pitchSemitones);
    }
    else
    {
        pitchText = juce::String(pitchSemitones);
    }
    
    pitchValueLabel.setText(pitchText, juce::dontSendNotification);
    
    // Color coding based on pitch
    if (pitchSemitones == 0)
    {
        pitchValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    }
    else if (pitchSemitones < 0)
    {
        pitchValueLabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);
    }
    else
    {
        pitchValueLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    }
}

void TransportBar::updatePositionLabels()
{
    positionLabel.setText(formatTime(currentPosition), juce::dontSendNotification);
    durationLabel.setText("/ " + formatTime(totalDuration), juce::dontSendNotification);
}

juce::String TransportBar::formatTime(double seconds) const
{
    if (seconds < 0.0)
        seconds = 0.0;
        
    int minutes = static_cast<int>(seconds) / 60;
    double remainingSeconds = seconds - (minutes * 60);
    
    return juce::String::formatted("%02d:%04.1f", minutes, remainingSeconds);
}