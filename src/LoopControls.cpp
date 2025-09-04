#include "LoopControls.h"

LoopControls::LoopControls()
{
    // Setup loop enable button
    loopEnabledButton.setButtonText("Loop");
    loopEnabledButton.setToggleState(false, juce::dontSendNotification);
    loopEnabledButton.onClick = [this]() { onLoopEnabledToggled(); };
    addAndMakeVisible(loopEnabledButton);
    
    // Setup quick loop operation buttons
    halfLoopButton.setButtonText("×½");
    halfLoopButton.onClick = [this]() { onHalfLoopClicked(); };
    addAndMakeVisible(halfLoopButton);
    
    doubleLoopButton.setButtonText("×2");
    doubleLoopButton.onClick = [this]() { onDoubleLoopClicked(); };
    addAndMakeVisible(doubleLoopButton);
    
    shortenButton.setButtonText("-Bar");
    shortenButton.onClick = [this]() { onShortenLoopClicked(); };
    addAndMakeVisible(shortenButton);
    
    extendButton.setButtonText("+Bar");
    extendButton.onClick = [this]() { onExtendLoopClicked(); };
    addAndMakeVisible(extendButton);
    
    // Setup snap-to-beat buttons
    snapStartToBeatButton.setButtonText("◄Beat");
    snapStartToBeatButton.onClick = [this]() { onSnapStartToBeatClicked(); };
    addAndMakeVisible(snapStartToBeatButton);
    
    snapEndToBeatButton.setButtonText("Beat►");
    snapEndToBeatButton.onClick = [this]() { onSnapEndToBeatClicked(); };
    addAndMakeVisible(snapEndToBeatButton);
    
    snapBothToBeatsButton.setButtonText("◄►Beat");
    snapBothToBeatsButton.onClick = [this]() { onSnapBothToBeatsClicked(); };
    addAndMakeVisible(snapBothToBeatsButton);
    
    // Setup loop here button
    loopHereButton.setButtonText("Loop Here");
    loopHereButton.onClick = [this]() { onLoopHereClicked(); };
    addAndMakeVisible(loopHereButton);
    
    updateButtonStates();
}

LoopControls::~LoopControls() = default;

void LoopControls::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff3a3a3a));
    
    // Border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 1);
    
    // Section headers
    auto bounds = getLocalBounds().reduced(4);
    
    g.setColour(juce::Colours::lightgrey);
    g.setFont(12.0f);
    
    // Draw section labels
    auto topRow = bounds.removeFromTop(20);
    g.drawText("Loop Controls", topRow.removeFromLeft(bounds.getWidth() / 3), 
               juce::Justification::centred, true);
    g.drawText("Quick Adjust", topRow.removeFromLeft(bounds.getWidth() / 2), 
               juce::Justification::centred, true);
    g.drawText("Snap to Beat", topRow, 
               juce::Justification::centred, true);
}

void LoopControls::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    bounds.removeFromTop(20); // Space for headers
    
    int buttonHeight = 30;
    int spacing = 4;
    
    // First row: Loop enable and quick operations
    auto firstRow = bounds.removeFromTop(buttonHeight);
    
    // Loop enable button takes 1/3 of width
    int sectionWidth = firstRow.getWidth() / 3;
    loopEnabledButton.setBounds(firstRow.removeFromLeft(sectionWidth).reduced(spacing));
    
    // Quick operations take remaining 2/3
    auto quickSection = firstRow.reduced(spacing);
    int quickButtonWidth = quickSection.getWidth() / 4;
    
    halfLoopButton.setBounds(quickSection.removeFromLeft(quickButtonWidth).reduced(spacing / 2));
    doubleLoopButton.setBounds(quickSection.removeFromLeft(quickButtonWidth).reduced(spacing / 2));
    shortenButton.setBounds(quickSection.removeFromLeft(quickButtonWidth).reduced(spacing / 2));
    extendButton.setBounds(quickSection.removeFromLeft(quickButtonWidth).reduced(spacing / 2));
    
    bounds.removeFromTop(spacing);
    
    // Second row: Snap operations and loop here
    auto secondRow = bounds.removeFromTop(buttonHeight);
    
    // Snap buttons take 2/3 of width
    auto snapSection = secondRow.removeFromLeft(sectionWidth * 2).reduced(spacing);
    int snapButtonWidth = snapSection.getWidth() / 3;
    
    snapStartToBeatButton.setBounds(snapSection.removeFromLeft(snapButtonWidth).reduced(spacing / 2));
    snapEndToBeatButton.setBounds(snapSection.removeFromLeft(snapButtonWidth).reduced(spacing / 2));
    snapBothToBeatsButton.setBounds(snapSection.removeFromLeft(snapButtonWidth).reduced(spacing / 2));
    
    // Loop here button takes remaining 1/3
    loopHereButton.setBounds(secondRow.reduced(spacing));
}

void LoopControls::setLoopEnabled(bool enabled)
{
    loopEnabled = enabled;
    loopEnabledButton.setToggleState(enabled, juce::dontSendNotification);
    updateButtonStates();
}

void LoopControls::setLoopPoints(double startSeconds, double endSeconds)
{
    loopStartSeconds = startSeconds;
    loopEndSeconds = endSeconds;
    updateButtonStates();
}

void LoopControls::setCurrentPosition(double positionSeconds)
{
    currentPositionSeconds = positionSeconds;
}

void LoopControls::setAnalysisResults(const AnalysisResult& results)
{
    std::lock_guard<std::mutex> lock(analysisMutex);
    currentAnalysis = results;
    updateButtonStates();
}

void LoopControls::setTotalDuration(double durationSeconds)
{
    totalDurationSeconds = durationSeconds;
    updateButtonStates();
}

void LoopControls::updateButtonStates()
{
    bool hasValidLoop = (loopEndSeconds > loopStartSeconds) && (totalDurationSeconds > 0.0);
    bool hasAnalysis = false;
    
    {
        std::lock_guard<std::mutex> lock(analysisMutex);
        hasAnalysis = currentAnalysis.isValid && !currentAnalysis.beats.empty();
    }
    
    // Enable/disable buttons based on state
    halfLoopButton.setEnabled(hasValidLoop);
    doubleLoopButton.setEnabled(hasValidLoop);
    shortenButton.setEnabled(hasValidLoop && hasAnalysis);
    extendButton.setEnabled(hasValidLoop && hasAnalysis);
    
    snapStartToBeatButton.setEnabled(hasValidLoop && hasAnalysis);
    snapEndToBeatButton.setEnabled(hasValidLoop && hasAnalysis);
    snapBothToBeatsButton.setEnabled(hasValidLoop && hasAnalysis);
    
    loopHereButton.setEnabled(totalDurationSeconds > 0.0);
    
    // Update loop enable button color
    if (loopEnabled)
    {
        loopEnabledButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    }
    else
    {
        loopEnabledButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    }
}

double LoopControls::findNearestBeat(double timeSeconds) const
{
    std::lock_guard<std::mutex> lock(analysisMutex);
    
    if (!currentAnalysis.isValid || currentAnalysis.beats.empty())
        return timeSeconds;
    
    double nearestBeat = currentAnalysis.beats[0];
    double minDistance = std::abs(timeSeconds - nearestBeat);
    
    for (double beatTime : currentAnalysis.beats)
    {
        double distance = std::abs(timeSeconds - beatTime);
        if (distance < minDistance)
        {
            minDistance = distance;
            nearestBeat = beatTime;
        }
    }
    
    return nearestBeat;
}

double LoopControls::getBarDuration() const
{
    std::lock_guard<std::mutex> lock(analysisMutex);
    
    if (!currentAnalysis.isValid || currentAnalysis.bpm <= 0.0)
        return 2.0; // Default 2-second bar
    
    // Assume 4/4 time signature (4 beats per bar)
    double beatsPerSecond = currentAnalysis.bpm / 60.0;
    return 4.0 / beatsPerSecond;
}

std::vector<double> LoopControls::getBeatsInRange(double startTime, double endTime) const
{
    std::lock_guard<std::mutex> lock(analysisMutex);
    std::vector<double> beatsInRange;
    
    if (!currentAnalysis.isValid || currentAnalysis.beats.empty())
        return beatsInRange;
    
    for (double beatTime : currentAnalysis.beats)
    {
        if (beatTime >= startTime && beatTime <= endTime)
        {
            beatsInRange.push_back(beatTime);
        }
    }
    
    return beatsInRange;
}

void LoopControls::onLoopEnabledToggled()
{
    loopEnabled = !loopEnabled;
    updateButtonStates();
    
    if (onLoopEnabledChanged)
        onLoopEnabledChanged(loopEnabled);
}

void LoopControls::onHalfLoopClicked()
{
    double loopDuration = loopEndSeconds - loopStartSeconds;
    double newDuration = loopDuration * 0.5;
    double newEnd = loopStartSeconds + newDuration;
    
    if (newEnd > loopStartSeconds && newEnd <= totalDurationSeconds)
    {
        loopEndSeconds = newEnd;
        if (onLoopPointsChanged)
            onLoopPointsChanged(loopStartSeconds, loopEndSeconds);
    }
}

void LoopControls::onDoubleLoopClicked()
{
    double loopDuration = loopEndSeconds - loopStartSeconds;
    double newDuration = loopDuration * 2.0;
    double newEnd = loopStartSeconds + newDuration;
    
    if (newEnd <= totalDurationSeconds)
    {
        loopEndSeconds = newEnd;
        if (onLoopPointsChanged)
            onLoopPointsChanged(loopStartSeconds, loopEndSeconds);
    }
}

void LoopControls::onShortenLoopClicked()
{
    double barDuration = getBarDuration();
    double newEnd = loopEndSeconds - barDuration;
    
    if (newEnd > loopStartSeconds + 0.1) // Minimum 0.1 second loop
    {
        loopEndSeconds = newEnd;
        if (onLoopPointsChanged)
            onLoopPointsChanged(loopStartSeconds, loopEndSeconds);
    }
}

void LoopControls::onExtendLoopClicked()
{
    double barDuration = getBarDuration();
    double newEnd = loopEndSeconds + barDuration;
    
    if (newEnd <= totalDurationSeconds)
    {
        loopEndSeconds = newEnd;
        if (onLoopPointsChanged)
            onLoopPointsChanged(loopStartSeconds, loopEndSeconds);
    }
}

void LoopControls::onSnapStartToBeatClicked()
{
    double snappedStart = findNearestBeat(loopStartSeconds);
    
    if (snappedStart < loopEndSeconds - 0.1) // Maintain minimum loop duration
    {
        loopStartSeconds = snappedStart;
        if (onLoopPointsChanged)
            onLoopPointsChanged(loopStartSeconds, loopEndSeconds);
    }
}

void LoopControls::onSnapEndToBeatClicked()
{
    double snappedEnd = findNearestBeat(loopEndSeconds);
    
    if (snappedEnd > loopStartSeconds + 0.1) // Maintain minimum loop duration
    {
        loopEndSeconds = snappedEnd;
        if (onLoopPointsChanged)
            onLoopPointsChanged(loopStartSeconds, loopEndSeconds);
    }
}

void LoopControls::onSnapBothToBeatsClicked()
{
    double snappedStart = findNearestBeat(loopStartSeconds);
    double snappedEnd = findNearestBeat(loopEndSeconds);
    
    if (snappedEnd > snappedStart + 0.1) // Maintain minimum loop duration
    {
        loopStartSeconds = snappedStart;
        loopEndSeconds = snappedEnd;
        if (onLoopPointsChanged)
            onLoopPointsChanged(loopStartSeconds, loopEndSeconds);
    }
}

void LoopControls::onLoopHereClicked()
{
    if (onLoopToCurrentPosition)
        onLoopToCurrentPosition();
}