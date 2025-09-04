#include "MainWindow_minimal.h"

// MainWindow Implementation
MainWindow::MainWindow(juce::String name)
    : DocumentWindow(name,
                     juce::Desktop::getInstance().getDefaultLookAndFeel()
                         .findColour(juce::ResizableWindow::backgroundColourId),
                     DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    
    mainComponent = std::make_unique<MainComponent>();
    setContentOwned(mainComponent.release(), true);

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
#else
    setResizable(true, true);
    centreWithSize(800, 600);
#endif

    setVisible(true);
}

MainWindow::~MainWindow() = default;

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

// MainComponent Implementation - Full GUI with Audio Engine
MainComponent::MainComponent()
{
    setSize(900, 700);

    // Initialize audio engine
    audioEngine = std::make_unique<SimpleAudioEngine>();
    audioEngine->initialize();

    // Start UI update timer
    startTimer(50); // Update UI 20 times per second

    // Title
    titleLabel.setText("Audio Practice Looper", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    // File loading section
    fileGroup.setText("Audio File");
    addAndMakeVisible(fileGroup);
    
    loadFileButton.setButtonText("Load Audio File...");
    loadFileButton.onClick = [this]()
    {
        loadAudioFile();
    };
    addAndMakeVisible(loadFileButton);
    
    fileStatusLabel.setText("No file loaded", juce::dontSendNotification);
    fileStatusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(fileStatusLabel);

    // Transport controls (improved)
    transportGroup.setText("Transport");
    addAndMakeVisible(transportGroup);
    
    // Combined play/pause button (Song Master style)
    playPauseButton.setButtonText("▶");  // Play icon
    playPauseButton.onClick = [this]()
    {
        if (audioEngine && audioEngine->isFileLoaded())
        {
            if (audioEngine->isPlaying())
            {
                audioEngine->pause();
                playPauseButton.setButtonText("▶");
                isCurrentlyPlaying = false;
                statusLabel.setText("Paused", juce::dontSendNotification);
            }
            else
            {
                audioEngine->play();
                playPauseButton.setButtonText("⏸");  // Pause icon
                isCurrentlyPlaying = true;
                statusLabel.setText("Playing", juce::dontSendNotification);
            }
        }
        else
        {
            statusLabel.setText("No file loaded", juce::dontSendNotification);
        }
    };
    addAndMakeVisible(playPauseButton);
    
    stopButton.setButtonText("⏹");  // Stop icon
    stopButton.onClick = [this]()
    {
        if (audioEngine)
        {
            audioEngine->stop();
            playPauseButton.setButtonText("▶");
            isCurrentlyPlaying = false;
            statusLabel.setText("Stopped", juce::dontSendNotification);
        }
    };
    addAndMakeVisible(stopButton);
    
    positionLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    positionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(positionLabel);

    // Tempo/Pitch controls
    effectsGroup.setText("Audio Effects");
    addAndMakeVisible(effectsGroup);
    
    tempoLabel.setText("Tempo:", juce::dontSendNotification);
    addAndMakeVisible(tempoLabel);
    
    tempoSlider.setRange(25.0, 200.0, 1.0);
    tempoSlider.setValue(100.0);
    tempoSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    tempoSlider.onValueChange = [this]() { 
        updateTempoLabel();
        if (audioEngine)
        {
            float ratio = (float)tempoSlider.getValue() / 100.0f;
            audioEngine->setTempoRatio(ratio);
        }
    };
    addAndMakeVisible(tempoSlider);
    
    tempoValueLabel.setText("100%", juce::dontSendNotification);
    addAndMakeVisible(tempoValueLabel);
    
    pitchLabel.setText("Pitch:", juce::dontSendNotification);
    addAndMakeVisible(pitchLabel);
    
    pitchSlider.setRange(-12.0, 12.0, 0.1);
    pitchSlider.setValue(0.0);
    pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    pitchSlider.onValueChange = [this]() { 
        updatePitchLabel();
        if (audioEngine)
        {
            int semitones = (int)pitchSlider.getValue();
            audioEngine->setPitchSemitones(semitones);
        }
    };
    addAndMakeVisible(pitchSlider);
    
    pitchValueLabel.setText("0 st", juce::dontSendNotification);
    addAndMakeVisible(pitchValueLabel);

    // Professional Loop Controls (Song Master Style)
    loopGroup.setText("Loop Controls");
    addAndMakeVisible(loopGroup);
    
    // Single A/B button that toggles between setting A, then B, then Clear
    abLoopButton.setButtonText("Set A");
    abLoopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff004080));
    abLoopButton.onClick = [this]()
    {
        if (!audioEngine || !audioEngine->isFileLoaded())
        {
            statusLabel.setText("No file loaded", juce::dontSendNotification);
            return;
        }
        
        switch (currentLoopState)
        {
            case NoLoop:
                audioEngine->setLoopAPoint();
                currentLoopState = HasA;
                abLoopButton.setButtonText("Set B");
                abLoopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff806000));
                statusLabel.setText("A point set - click again for B point", juce::dontSendNotification);
                break;
                
            case HasA:
                audioEngine->setLoopBPoint();
                currentLoopState = HasAB;
                abLoopButton.setButtonText("Clear");
                abLoopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff800000));
                statusLabel.setText("Loop A-B active", juce::dontSendNotification);
                break;
                
            case HasAB:
                audioEngine->clearLoop();
                currentLoopState = NoLoop;
                abLoopButton.setButtonText("Set A");
                abLoopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff004080));
                statusLabel.setText("Loop cleared - playback continues", juce::dontSendNotification);
                break;
        }
        updateLoopInfo();
    };
    addAndMakeVisible(abLoopButton);
    
    // Fine-tuning jog buttons for A/B points
    jogALeftButton.setButtonText("A◀");
    jogALeftButton.onClick = [this]() { 
        if (audioEngine) {
            audioEngine->jogLoopStart(-0.005);
            updateLoopInfo();
        }
    };
    addAndMakeVisible(jogALeftButton);
    
    jogARightButton.setButtonText("A▶");
    jogARightButton.onClick = [this]() { 
        if (audioEngine) {
            audioEngine->jogLoopStart(0.005);
            updateLoopInfo();
        }
    };
    addAndMakeVisible(jogARightButton);
    
    jogBLeftButton.setButtonText("B◀");
    jogBLeftButton.onClick = [this]() { 
        if (audioEngine) {
            audioEngine->jogLoopEnd(-0.005);
            updateLoopInfo();
        }
    };
    addAndMakeVisible(jogBLeftButton);
    
    jogBRightButton.setButtonText("B▶");
    jogBRightButton.onClick = [this]() { 
        if (audioEngine) {
            audioEngine->jogLoopEnd(0.005);
            updateLoopInfo();
        }
    };
    addAndMakeVisible(jogBRightButton);
    
    // Loop length controls
    doubleLoopButton.setButtonText("2×");
    doubleLoopButton.onClick = [this]() { 
        if (audioEngine) {
            audioEngine->doubleLoopLength();
            updateLoopInfo();
            statusLabel.setText("Loop length doubled", juce::dontSendNotification);
        }
    };
    addAndMakeVisible(doubleLoopButton);
    
    halveLoopButton.setButtonText("½×");
    halveLoopButton.onClick = [this]() { 
        if (audioEngine) {
            audioEngine->halveLoopLength();
            updateLoopInfo();
            statusLabel.setText("Loop length halved", juce::dontSendNotification);
        }
    };
    addAndMakeVisible(halveLoopButton);
    
    // Bar movement controls
    moveLoopBackButton.setButtonText("◀◀");
    moveLoopBackButton.onClick = [this]() { 
        if (audioEngine) {
            audioEngine->moveLoopRegionBackward();
            updateLoopInfo();
            statusLabel.setText("Loop moved backward", juce::dontSendNotification);
        }
    };
    addAndMakeVisible(moveLoopBackButton);
    
    moveLoopForwardButton.setButtonText("▶▶");
    moveLoopForwardButton.onClick = [this]() { 
        if (audioEngine) {
            audioEngine->moveLoopRegionForward();
            updateLoopInfo();
            statusLabel.setText("Loop moved forward", juce::dontSendNotification);
        }
    };
    addAndMakeVisible(moveLoopForwardButton);

    loopInfoLabel.setText("Loop: Not set", juce::dontSendNotification);
    loopInfoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(loopInfoLabel);

    // Recording controls
    recordingGroup.setText("Recording");
    addAndMakeVisible(recordingGroup);
    
    recordButton.setButtonText("Record");
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff990000));
    recordButton.onClick = [this]()
    {
        if (audioEngine)
        {
            if (audioEngine->isRecording())
            {
                audioEngine->stopRecording();
                recordButton.setButtonText("Record");
                recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff990000));
                recordingStatusLabel.setText("Recording stopped", juce::dontSendNotification);
            }
            else
            {
                audioEngine->startRecording();
                recordButton.setButtonText("Stop Rec");
                recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff009900));
                recordingStatusLabel.setText("Recording...", juce::dontSendNotification);
            }
        }
    };
    addAndMakeVisible(recordButton);
    
    inputMonitorButton.setButtonText("Monitor Input");
    inputMonitorButton.onClick = [this]()
    {
        if (audioEngine)
        {
            bool enabled = inputMonitorButton.getToggleState();
            audioEngine->setInputMonitoring(enabled);
            statusLabel.setText(enabled ? "Input monitoring enabled" : "Input monitoring disabled", juce::dontSendNotification);
        }
    };
    addAndMakeVisible(inputMonitorButton);
    
    saveRecordingButton.setButtonText("Save Recording...");
    saveRecordingButton.onClick = [this]()
    {
        saveRecording();
    };
    addAndMakeVisible(saveRecordingButton);
    
    recordingStatusLabel.setText("Ready to record", juce::dontSendNotification);
    recordingStatusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(recordingStatusLabel);

    // Waveform area
    waveformArea.setPaintingIsUnclipped(true);
    addAndMakeVisible(waveformArea);

    // Settings Menu (Hamburger)
    settingsMenuButton.setButtonText("☰");
    settingsMenuButton.onClick = [this]() { showSettingsMenu(); };
    addAndMakeVisible(settingsMenuButton);

    // Status
    statusLabel.setText("Ready - Load an audio file to begin", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setFont(juce::Font(14.0f));
    addAndMakeVisible(statusLabel);
}

MainComponent::~MainComponent()
{
    if (audioEngine)
    {
        audioEngine->shutdown();
    }
}

void MainComponent::paint(juce::Graphics& g)
{
    // DAW-style dark background
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Draw waveform area background
    auto waveformBounds = waveformArea.getBounds();
    if (!waveformBounds.isEmpty())
    {
        // Dark waveform background (like Pro Tools, Logic, etc.)
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRect(waveformBounds);
        
        // Border
        g.setColour(juce::Colour(0xff3a3a3a));
        g.drawRect(waveformBounds, 1);
        
        drawWaveformPlaceholder(g, waveformBounds);
    }
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Top title bar (like DAW title) with settings menu
    auto titleArea = bounds.removeFromTop(40);
    settingsMenuButton.setBounds(titleArea.removeFromRight(40).reduced(5));
    titleLabel.setBounds(titleArea);
    
    // Main content area
    bounds.reduce(10, 5);
    
    // File section at top (like DAW file/project area)
    auto fileArea = bounds.removeFromTop(80);
    fileGroup.setBounds(fileArea);
    fileArea.reduce(10, 15);
    loadFileButton.setBounds(fileArea.removeFromTop(30).reduced(5));
    fileStatusLabel.setBounds(fileArea);
    
    bounds.removeFromTop(5);
    
    // Middle section - Transport and waveform area (main DAW-style layout)
    auto middleSection = bounds.removeFromTop(120);
    
    // Transport controls on left (Song Master style)
    auto transportArea = middleSection.removeFromLeft(150);
    transportGroup.setBounds(transportArea);
    transportArea.reduce(10, 15);
    
    auto transportButtons = transportArea.removeFromTop(35);
    playPauseButton.setBounds(transportButtons.removeFromLeft(60).reduced(2));
    stopButton.setBounds(transportButtons.removeFromLeft(60).reduced(2));
    
    positionLabel.setBounds(transportArea);
    
    // Waveform area (main DAW workspace)
    middleSection.removeFromLeft(10);
    waveformArea.setBounds(middleSection);
    
    bounds.removeFromTop(10);
    
    // Bottom section - Controls area (like DAW mixer/effects section)
    auto controlsHeight = bounds.getHeight() - 40; // Leave room for status
    auto controlsArea = bounds.removeFromTop(controlsHeight);
    
    // Effects on left side
    auto effectsArea = controlsArea.removeFromLeft(300);
    effectsGroup.setBounds(effectsArea);
    effectsArea.reduce(10, 15);
    
    // Tempo controls
    auto tempoArea = effectsArea.removeFromTop(35);
    tempoLabel.setBounds(tempoArea.removeFromLeft(50));
    tempoSlider.setBounds(tempoArea.removeFromLeft(180));
    tempoValueLabel.setBounds(tempoArea);
    
    effectsArea.removeFromTop(5);
    
    // Pitch controls
    auto pitchArea = effectsArea.removeFromTop(35);
    pitchLabel.setBounds(pitchArea.removeFromLeft(50));
    pitchSlider.setBounds(pitchArea.removeFromLeft(180));
    pitchValueLabel.setBounds(pitchArea);
    
    // Professional Loop Controls (Song Master style layout)
    controlsArea.removeFromLeft(10);
    auto loopArea = controlsArea.removeFromLeft(400);
    loopGroup.setBounds(loopArea);
    loopArea.reduce(10, 15);
    
    // Top row: Main A/B button and jog controls
    auto topRow = loopArea.removeFromTop(30);
    abLoopButton.setBounds(topRow.removeFromLeft(80).reduced(2));
    topRow.removeFromLeft(10);
    
    // A point jog buttons
    jogALeftButton.setBounds(topRow.removeFromLeft(35).reduced(2));
    jogARightButton.setBounds(topRow.removeFromLeft(35).reduced(2));
    topRow.removeFromLeft(10);
    
    // B point jog buttons
    jogBLeftButton.setBounds(topRow.removeFromLeft(35).reduced(2));
    jogBRightButton.setBounds(topRow.removeFromLeft(35).reduced(2));
    
    loopArea.removeFromTop(5);
    
    // Middle row: Loop length and movement controls
    auto middleRow = loopArea.removeFromTop(30);
    halveLoopButton.setBounds(middleRow.removeFromLeft(40).reduced(2));
    doubleLoopButton.setBounds(middleRow.removeFromLeft(40).reduced(2));
    middleRow.removeFromLeft(20);
    moveLoopBackButton.setBounds(middleRow.removeFromLeft(40).reduced(2));
    moveLoopForwardButton.setBounds(middleRow.removeFromLeft(40).reduced(2));
    
    loopArea.removeFromTop(5);
    
    // Bottom: Loop info display
    loopInfoLabel.setBounds(loopArea.removeFromTop(25));
    
    // Recording controls on right side
    controlsArea.removeFromLeft(10);
    recordingGroup.setBounds(controlsArea);
    controlsArea.reduce(10, 15);
    
    recordButton.setBounds(controlsArea.removeFromTop(30));
    controlsArea.removeFromTop(5);
    inputMonitorButton.setBounds(controlsArea.removeFromTop(25));
    controlsArea.removeFromTop(5);
    saveRecordingButton.setBounds(controlsArea.removeFromTop(30));
    controlsArea.removeFromTop(5);
    recordingStatusLabel.setBounds(controlsArea.removeFromTop(25));
    
    // Status bar at bottom (like DAW status bar)
    statusLabel.setBounds(bounds);
}

// Helper methods for updating labels
void MainComponent::updateTempoLabel()
{
    int tempo = (int)tempoSlider.getValue();
    tempoValueLabel.setText(juce::String(tempo) + "%", juce::dontSendNotification);
    statusLabel.setText("Tempo: " + juce::String(tempo) + "%", juce::dontSendNotification);
}

void MainComponent::updatePitchLabel()
{
    double pitch = pitchSlider.getValue();
    juce::String pitchText = juce::String(pitch, 1) + " st";
    pitchValueLabel.setText(pitchText, juce::dontSendNotification);
    statusLabel.setText("Pitch: " + pitchText, juce::dontSendNotification);
}

void MainComponent::timerCallback()
{
    updatePositionLabel();
    updateUIState();
}

void MainComponent::updatePositionLabel()
{
    if (!audioEngine || !audioEngine->isFileLoaded())
    {
        positionLabel.setText("00:00 / 00:00", juce::dontSendNotification);
        return;
    }
    
    double position = audioEngine->getPosition();
    double duration = audioEngine->getDuration();
    
    auto formatTime = [](double seconds) -> juce::String
    {
        int mins = (int)(seconds / 60.0);
        int secs = (int)(seconds - mins * 60.0);
        return juce::String::formatted("%02d:%02d", mins, secs);
    };
    
    juce::String posText = formatTime(position) + " / " + formatTime(duration);
    positionLabel.setText(posText, juce::dontSendNotification);
}

void MainComponent::updateUIState()
{
    if (!audioEngine)
        return;
        
    bool fileLoaded = audioEngine->isFileLoaded();
    
    playPauseButton.setEnabled(fileLoaded);
    stopButton.setEnabled(fileLoaded);
    abLoopButton.setEnabled(fileLoaded);
    
    // Enable/disable jog buttons based on loop state
    jogALeftButton.setEnabled(fileLoaded && currentLoopState >= HasA);
    jogARightButton.setEnabled(fileLoaded && currentLoopState >= HasA);
    jogBLeftButton.setEnabled(fileLoaded && currentLoopState >= HasAB);
    jogBRightButton.setEnabled(fileLoaded && currentLoopState >= HasAB);
    
    // Enable loop manipulation buttons only when we have both A and B points
    doubleLoopButton.setEnabled(fileLoaded && currentLoopState == HasAB);
    halveLoopButton.setEnabled(fileLoaded && currentLoopState == HasAB);
    moveLoopBackButton.setEnabled(fileLoaded && currentLoopState == HasAB);
    moveLoopForwardButton.setEnabled(fileLoaded && currentLoopState == HasAB);
    
    tempoSlider.setEnabled(fileLoaded);
    pitchSlider.setEnabled(fileLoaded);
}

void MainComponent::updateLoopInfo()
{
    if (!audioEngine || !audioEngine->isFileLoaded())
    {
        loopInfoLabel.setText("Loop: Not set", juce::dontSendNotification);
        return;
    }
    
    double start = audioEngine->getLoopStart();
    double end = audioEngine->getLoopEnd();
    
    auto formatTime = [](double seconds) -> juce::String
    {
        int mins = (int)(seconds / 60.0);
        int secs = (int)(seconds - mins * 60.0);
        return juce::String::formatted("%02d:%02d", mins, secs);
    };
    
    juce::String loopText = "Loop: " + formatTime(start) + " - " + formatTime(end);
    loopInfoLabel.setText(loopText, juce::dontSendNotification);
}

void MainComponent::loadAudioFile()
{
    if (!audioEngine)
        return;
        
    juce::FileChooser chooser("Select an audio file...",
                             juce::File::getSpecialLocation(juce::File::userMusicDirectory),
                             "*.wav;*.aiff;*.mp3;*.m4a;*.flac");
    
    if (chooser.browseForFileToOpen())
    {
        juce::File file = chooser.getResult();
        
        if (audioEngine->loadAudioFile(file))
        {
            fileStatusLabel.setText("Loaded: " + file.getFileName(), juce::dontSendNotification);
            statusLabel.setText("File loaded successfully", juce::dontSendNotification);
            
            // Update loop info with default full-file loop
            updateLoopInfo();
        }
        else
        {
            fileStatusLabel.setText("Failed to load file", juce::dontSendNotification);
            statusLabel.setText("Error: Could not load audio file", juce::dontSendNotification);
        }
    }
}

void MainComponent::saveRecording()
{
    if (!audioEngine)
        return;
        
    juce::FileChooser chooser("Save recording as...",
                             juce::File::getSpecialLocation(juce::File::userMusicDirectory),
                             "*.wav");
    
    if (chooser.browseForFileToSave(true))
    {
        juce::File file = chooser.getResult();
        
        // Ensure .wav extension
        if (!file.hasFileExtension(".wav"))
        {
            file = file.withFileExtension(".wav");
        }
        
        audioEngine->saveRecording(file);
        statusLabel.setText("Recording saved to " + file.getFileName(), juce::dontSendNotification);
        recordingStatusLabel.setText("Recording saved", juce::dontSendNotification);
    }
}

void MainComponent::drawWaveformPlaceholder(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // DAW-style waveform visualization
    bounds.reduce(5, 5);
    
    if (bounds.getWidth() < 10) return;
    
    // Draw placeholder waveform (like Pro Tools/Logic style)
    g.setColour(juce::Colour(0xff4a9eff)); // Blue waveform color
    
    auto center = bounds.getCentreY();
    auto width = bounds.getWidth();
    
    // Draw fake stereo waveform
    for (int x = 0; x < width; x += 2)
    {
        // Create realistic waveform shape using sine waves with random variation
        float phase1 = (float)x * 0.02f;
        float phase2 = (float)x * 0.015f;
        float amplitude1 = std::sin(phase1) * std::sin(phase1 * 0.1f) * 0.7f;
        float amplitude2 = std::sin(phase2) * std::cos(phase2 * 0.08f) * 0.6f;
        
        // Add some random variation
        amplitude1 *= (0.7f + 0.6f * std::sin(x * 0.001f));
        amplitude2 *= (0.8f + 0.4f * std::cos(x * 0.0015f));
        
        // Top channel (L)
        int y1_top = center - 15 - (int)(amplitude1 * 20);
        int y2_top = center - 15;
        
        // Bottom channel (R) 
        int y1_bottom = center + 15;
        int y2_bottom = center + 15 + (int)(amplitude2 * 20);
        
        g.drawLine(bounds.getX() + x, y1_top, bounds.getX() + x, y2_top, 1.0f);
        g.drawLine(bounds.getX() + x, y1_bottom, bounds.getX() + x, y2_bottom, 1.0f);
    }
    
    // Draw center line
    g.setColour(juce::Colour(0xff666666));
    g.drawLine(bounds.getX(), center, bounds.getRight(), center, 1.0f);
    
    // Draw playback position line (placeholder)
    int playbackPos = bounds.getX() + (width / 3); // Fake position at 1/3
    g.setColour(juce::Colour(0xffff6b6b)); // Red playback line
    g.drawLine(playbackPos, bounds.getY(), playbackPos, bounds.getBottom(), 2.0f);
    
    // Draw loop markers (placeholder)
    int loopStart = bounds.getX() + (width / 4);
    int loopEnd = bounds.getX() + (3 * width / 4);
    g.setColour(juce::Colour(0xffffe66d)); // Yellow loop markers
    g.drawLine(loopStart, bounds.getY(), loopStart, bounds.getBottom(), 1.5f);
    g.drawLine(loopEnd, bounds.getY(), loopEnd, bounds.getBottom(), 1.5f);
    
    // Draw text overlay
    g.setColour(juce::Colour(0xff888888));
    g.setFont(14.0f);
    g.drawText("Load audio file to see waveform", bounds.reduced(10), juce::Justification::centred);
}

void MainComponent::updateABButtonState()
{
    // This will be called by the timer to sync button state with engine
    // Implementation can check engine state and update button appearance
}

void MainComponent::showSettingsMenu()
{
    // Create a popup menu for settings
    juce::PopupMenu menu;
    
    juce::PopupMenu edgeBleedMenu;
    edgeBleedMenu.addItem(1001, "0ms (no bleed)", true, audioEngine && audioEngine->getEdgeBleedMs() == 0);
    edgeBleedMenu.addItem(1002, "5ms", true, audioEngine && audioEngine->getEdgeBleedMs() == 5);
    edgeBleedMenu.addItem(1003, "10ms", true, audioEngine && audioEngine->getEdgeBleedMs() == 10);
    edgeBleedMenu.addItem(1004, "20ms", true, audioEngine && audioEngine->getEdgeBleedMs() == 20);
    
    menu.addSubMenu("Edge Bleed", edgeBleedMenu);
    menu.addSeparator();
    menu.addItem(2001, "Snap to Grid", true, audioEngine && audioEngine->getSnapToGrid());
    menu.addSeparator();
    menu.addItem(3001, "About Audio Practice Looper");
    
    auto result = menu.show();
    
    if (result >= 1001 && result <= 1004)
    {
        // Handle edge bleed settings
        int bleedMs = 0;
        switch (result)
        {
            case 1001: bleedMs = 0; break;
            case 1002: bleedMs = 5; break;
            case 1003: bleedMs = 10; break;
            case 1004: bleedMs = 20; break;
        }
        
        if (audioEngine)
        {
            audioEngine->setEdgeBleedMs(bleedMs);
            statusLabel.setText("Edge bleed set to " + juce::String(bleedMs) + "ms", juce::dontSendNotification);
        }
    }
    else if (result == 2001)
    {
        // Toggle snap to grid
        if (audioEngine)
        {
            bool newSnap = !audioEngine->getSnapToGrid();
            audioEngine->setSnapToGrid(newSnap);
            statusLabel.setText(newSnap ? "Snap to grid enabled" : "Snap to grid disabled", juce::dontSendNotification);
        }
    }
    else if (result == 3001)
    {
        // Show about dialog
        juce::AlertWindow::showMessageBox(juce::AlertWindow::InfoIcon,
                                        "About Audio Practice Looper",
                                        "Professional audio practice tool with advanced loop controls.\n\n"
                                        "Features:\n"
                                        "• A/B loop points with fine-tuning\n"
                                        "• Loop length doubling/halving\n"
                                        "• Loop region movement\n"
                                        "• Audio recording and monitoring\n"
                                        "• Edge bleed control for seamless loops\n\n"
                                        "Built with JUCE Framework");
    }
}