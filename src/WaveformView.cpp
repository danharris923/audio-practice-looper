#include "WaveformView.h"

WaveformView::WaveformView()
{
    // Start timer for regular updates (30 FPS)
    startTimer(33);
}

WaveformView::~WaveformView()
{
    stopTimer();
}

void WaveformView::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    
    // Background
    g.fillAll(juce::Colours::black);
    
    if (waveformData.empty() || totalDuration <= 0.0)
    {
        // No data loaded - show placeholder
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(area);
        
        g.setColour(juce::Colours::lightgrey);
        g.setFont(16.0f);
        g.drawText("Load an audio file to see waveform", area, 
                  juce::Justification::centred, true);
        return;
    }
    
    // Draw waveform
    drawWaveform(g, area);
    
    // Draw beat grid overlay
    drawBeatGrid(g, area);
    
    // Draw loop points
    if (loopEnabled.load())
    {
        drawLoopPoints(g, area);
    }
    
    // Draw playback position
    drawPlaybackPosition(g, area);
}

void WaveformView::resized()
{
    updateViewRange();
    
    // Regenerate display peaks if width changed
    if (getWidth() != lastWidth)
    {
        generateDisplayPeaks();
        lastWidth = getWidth();
    }
}

void WaveformView::mouseDown(const juce::MouseEvent& event)
{
    if (totalDuration <= 0.0)
        return;
        
    auto area = getLocalBounds();
    double clickTime = pixelToTime(event.x, area);
    
    // Check if clicking near loop points
    double loopStartTime = loopStart.load();
    double loopEndTime = loopEnd.load();
    
    int loopStartX = timeToPixel(loopStartTime, area);
    int loopEndX = timeToPixel(loopEndTime, area);
    
    const int dragThreshold = 8; // pixels
    
    if (loopEnabled.load() && std::abs(event.x - loopStartX) <= dragThreshold)
    {
        isDraggingLoopStart = true;
        mouseDownX = event.x;
    }
    else if (loopEnabled.load() && std::abs(event.x - loopEndX) <= dragThreshold)
    {
        isDraggingLoopEnd = true;
        mouseDownX = event.x;
    }
    else
    {
        // Regular seek
        if (onSeekRequested)
        {
            onSeekRequested(clickTime);
        }
    }
}

void WaveformView::mouseDrag(const juce::MouseEvent& event)
{
    if (totalDuration <= 0.0)
        return;
        
    auto area = getLocalBounds();
    double dragTime = pixelToTime(event.x, area);
    
    if (isDraggingLoopStart)
    {
        double endTime = loopEnd.load();
        double newStart = juce::jlimit(0.0, endTime - 0.1, dragTime);
        
        loopStart.store(newStart);
        
        if (onLoopPointsChanged)
        {
            onLoopPointsChanged(newStart, endTime);
        }
        repaint();
    }
    else if (isDraggingLoopEnd)
    {
        double startTime = loopStart.load();
        double newEnd = juce::jlimit(startTime + 0.1, totalDuration, dragTime);
        
        loopEnd.store(newEnd);
        
        if (onLoopPointsChanged)
        {
            onLoopPointsChanged(startTime, newEnd);
        }
        repaint();
    }
}

void WaveformView::setWaveformData(const std::vector<float>& audioData, double sampleRate, int channels)
{
    this->waveformData = audioData;
    this->sampleRate = sampleRate;
    this->numChannels = channels;
    this->totalDuration = audioData.size() / (sampleRate * channels);
    
    updateViewRange();
    generateDisplayPeaks();
    repaint();
}

void WaveformView::clearWaveformData()
{
    waveformData.clear();
    displayPeaks.clear();
    totalDuration = 0.0;
    playbackPosition.store(0.0);
    repaint();
}

void WaveformView::setAnalysisResults(const AnalysisResult& results)
{
    std::lock_guard<std::mutex> lock(analysisMutex);
    currentAnalysis = results;
    repaint();
}

void WaveformView::setPlaybackPosition(double positionSeconds)
{
    playbackPosition.store(positionSeconds);
}

void WaveformView::setTotalDuration(double durationSeconds)
{
    totalDuration = durationSeconds;
    updateViewRange();
    repaint();
}

void WaveformView::setLoopPoints(double startSeconds, double endSeconds)
{
    loopStart.store(startSeconds);
    loopEnd.store(endSeconds);
    repaint();
}

void WaveformView::setLoopEnabled(bool enabled)
{
    loopEnabled.store(enabled);
    repaint();
}

void WaveformView::setZoom(double newZoomFactor)
{
    zoomFactor = juce::jlimit(0.1, 100.0, newZoomFactor);
    updateViewRange();
    generateDisplayPeaks();
    repaint();
}

void WaveformView::setViewStart(double startSeconds)
{
    viewStartSeconds = juce::jlimit(0.0, totalDuration, startSeconds);
    updateViewRange();
    generateDisplayPeaks();
    repaint();
}

void WaveformView::timerCallback()
{
    // Regular repaint for playback position updates
    repaint();
}

void WaveformView::generateDisplayPeaks()
{
    int width = getWidth();
    if (width <= 0 || waveformData.empty() || totalDuration <= 0.0)
    {
        displayPeaks.clear();
        return;
    }
    
    displayPeaks.resize(width * 2); // Min and max for each pixel
    
    double viewDuration = viewEndSeconds - viewStartSeconds;
    double samplesPerPixel = (waveformData.size() / numChannels) * (viewDuration / totalDuration) / width;
    
    for (int x = 0; x < width; ++x)
    {
        double timeAtPixel = viewStartSeconds + (x * viewDuration / width);
        int startSample = static_cast<int>(timeAtPixel * sampleRate) * numChannels;
        int endSample = static_cast<int>((timeAtPixel + viewDuration / width) * sampleRate) * numChannels;
        
        endSample = juce::jmin(endSample, static_cast<int>(waveformData.size()));
        startSample = juce::jmax(startSample, 0);
        
        float minVal = 0.0f;
        float maxVal = 0.0f;
        
        // Find min/max in this pixel range
        for (int sample = startSample; sample < endSample; sample += numChannels)
        {
            // Mix down to mono for display
            float sampleValue = 0.0f;
            for (int ch = 0; ch < numChannels && sample + ch < static_cast<int>(waveformData.size()); ++ch)
            {
                sampleValue += waveformData[sample + ch];
            }
            sampleValue /= numChannels;
            
            minVal = juce::jmin(minVal, sampleValue);
            maxVal = juce::jmax(maxVal, sampleValue);
        }
        
        displayPeaks[x * 2] = minVal;
        displayPeaks[x * 2 + 1] = maxVal;
    }
}

void WaveformView::drawWaveform(juce::Graphics& g, juce::Rectangle<int> area)
{
    if (displayPeaks.empty())
        return;
        
    g.setColour(getWaveformColour());
    
    int width = area.getWidth();
    int height = area.getHeight();
    int centerY = area.getCentreY();
    
    for (int x = 0; x < width && x * 2 + 1 < static_cast<int>(displayPeaks.size()); ++x)
    {
        float minVal = displayPeaks[x * 2];
        float maxVal = displayPeaks[x * 2 + 1];
        
        int minY = centerY + static_cast<int>(minVal * height * 0.4f);
        int maxY = centerY + static_cast<int>(maxVal * height * 0.4f);
        
        if (minY != maxY)
        {
            g.drawVerticalLine(area.getX() + x, static_cast<float>(maxY), static_cast<float>(minY));
        }
        else
        {
            g.setPixel(area.getX() + x, minY);
        }
    }
}

void WaveformView::drawBeatGrid(juce::Graphics& g, juce::Rectangle<int> area)
{
    std::lock_guard<std::mutex> lock(analysisMutex);
    
    if (!currentAnalysis.isValid || currentAnalysis.beats.empty())
        return;
        
    g.setColour(juce::Colours::yellow.withAlpha(0.6f));
    
    for (double beatTime : currentAnalysis.beats)
    {
        if (beatTime >= viewStartSeconds && beatTime <= viewEndSeconds)
        {
            int x = timeToPixel(beatTime, area);
            g.drawVerticalLine(x, static_cast<float>(area.getY()), 
                             static_cast<float>(area.getBottom()));
        }
    }
    
    // Draw BPM info if available
    if (currentAnalysis.bpm > 0.0)
    {
        g.setColour(juce::Colours::yellow);
        g.setFont(12.0f);
        g.drawText("BPM: " + juce::String(currentAnalysis.bpm, 1), 
                  area.removeFromTop(20), juce::Justification::topRight);
    }
}

void WaveformView::drawLoopPoints(juce::Graphics& g, juce::Rectangle<int> area)
{
    double startTime = loopStart.load();
    double endTime = loopEnd.load();
    
    int startX = timeToPixel(startTime, area);
    int endX = timeToPixel(endTime, area);
    
    // Loop region highlight
    if (startX >= area.getX() && endX <= area.getRight())
    {
        auto loopArea = juce::Rectangle<int>(startX, area.getY(), 
                                           endX - startX, area.getHeight());
        g.setColour(juce::Colours::green.withAlpha(0.2f));
        g.fillRect(loopArea);
    }
    
    // Loop start marker
    g.setColour(juce::Colours::green);
    g.drawVerticalLine(startX, static_cast<float>(area.getY()), 
                      static_cast<float>(area.getBottom()));
    g.fillRect(startX - 3, area.getY(), 6, 10); // Top handle
    
    // Loop end marker
    g.drawVerticalLine(endX, static_cast<float>(area.getY()), 
                      static_cast<float>(area.getBottom()));
    g.fillRect(endX - 3, area.getY(), 6, 10); // Top handle
}

void WaveformView::drawPlaybackPosition(juce::Graphics& g, juce::Rectangle<int> area)
{
    double currentPos = playbackPosition.load();
    
    if (currentPos >= viewStartSeconds && currentPos <= viewEndSeconds)
    {
        int x = timeToPixel(currentPos, area);
        
        g.setColour(juce::Colours::white);
        g.drawVerticalLine(x, static_cast<float>(area.getY()), 
                          static_cast<float>(area.getBottom()));
        
        // Playhead triangle
        juce::Path triangle;
        triangle.addTriangle(x - 5.0f, area.getY(), 
                           x + 5.0f, area.getY(),
                           x, area.getY() + 10.0f);
        g.fillPath(triangle);
    }
}

double WaveformView::pixelToTime(int pixel, juce::Rectangle<int> area) const
{
    if (totalDuration <= 0.0)
        return 0.0;
        
    double relativeX = (pixel - area.getX()) / static_cast<double>(area.getWidth());
    return viewStartSeconds + relativeX * (viewEndSeconds - viewStartSeconds);
}

int WaveformView::timeToPixel(double time, juce::Rectangle<int> area) const
{
    if (totalDuration <= 0.0 || viewEndSeconds <= viewStartSeconds)
        return area.getX();
        
    double relativeTime = (time - viewStartSeconds) / (viewEndSeconds - viewStartSeconds);
    return area.getX() + static_cast<int>(relativeTime * area.getWidth());
}

void WaveformView::updateViewRange()
{
    if (totalDuration <= 0.0)
    {
        viewStartSeconds = 0.0;
        viewEndSeconds = 1.0;
        return;
    }
    
    double viewDuration = totalDuration / zoomFactor;
    
    viewEndSeconds = viewStartSeconds + viewDuration;
    
    // Clamp to valid range
    if (viewEndSeconds > totalDuration)
    {
        viewEndSeconds = totalDuration;
        viewStartSeconds = juce::jmax(0.0, viewEndSeconds - viewDuration);
    }
    
    if (viewStartSeconds < 0.0)
    {
        viewStartSeconds = 0.0;
        viewEndSeconds = juce::jmin(totalDuration, viewDuration);
    }
}

juce::Colour WaveformView::getWaveformColour() const
{
    return juce::Colours::cyan.withAlpha(0.8f);
}