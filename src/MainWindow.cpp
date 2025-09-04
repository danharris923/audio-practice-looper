#include "MainWindow.h"
#include "AudioEngine.h"

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
    centreWithSize(getWidth(), getHeight());
#endif

    setVisible(true);
}

MainWindow::~MainWindow() = default;

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

// MainComponent Implementation
MainComponent::MainComponent()
{
    setSize(800, 600);

    // Initialize audio engine
    audioEngine = std::make_unique<AudioEngine>();
    audioEngine->initialize();

    welcomeLabel.setText("Audio Practice Looper", juce::dontSendNotification);
    welcomeLabel.setJustificationType(juce::Justification::centred);
    welcomeLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    addAndMakeVisible(welcomeLabel);

    loadFileButton.setButtonText("Load Audio File");
    loadFileButton.onClick = [this]()
    {
        juce::FileChooser chooser("Select an audio file to load...",
                                 juce::File{}, 
                                 "*.wav;*.mp3;*.flac;*.aiff");
        
        if (chooser.browseForFileToOpen())
        {
            auto file = chooser.getResult();
            bool success = audioEngine->loadAudioFile(file);
            
            if (success)
            {
                juce::AlertWindow::showMessageBox(juce::AlertWindow::InfoIcon,
                                                 "Success",
                                                 "Audio file loaded: " + file.getFileName());
            }
            else
            {
                juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon,
                                                 "Error",
                                                 "Failed to load audio file (not yet implemented)");
            }
        }
    };
    addAndMakeVisible(loadFileButton);

    playPauseButton.setButtonText("Play");
    playPauseButton.onClick = [this]()
    {
        if (audioEngine->isPlaying())
        {
            audioEngine->pause();
            playPauseButton.setButtonText("Play");
        }
        else
        {
            audioEngine->play();
            playPauseButton.setButtonText("Pause");
        }
    };
    addAndMakeVisible(playPauseButton);

    tempoLabel.setText("Tempo: 100%", juce::dontSendNotification);
    tempoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(tempoLabel);

    tempoSlider.setRange(25.0, 200.0);
    tempoSlider.setValue(100.0);
    tempoSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80);
    tempoSlider.onValueChange = [this]()
    {
        int tempoPercent = static_cast<int>(tempoSlider.getValue());
        float tempoRatio = tempoPercent / 100.0f;
        
        audioEngine->setTempoRatio(tempoRatio);
        tempoLabel.setText("Tempo: " + juce::String(tempoPercent) + "%",
                          juce::dontSendNotification);
    };
    addAndMakeVisible(tempoSlider);

    pitchLabel.setText("Pitch: 0 semitones", juce::dontSendNotification);
    pitchLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(pitchLabel);

    pitchSlider.setRange(-12.0, 12.0);
    pitchSlider.setValue(0.0);
    pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80);
    pitchSlider.onValueChange = [this]()
    {
        int semitones = static_cast<int>(pitchSlider.getValue());
        audioEngine->setPitchSemitones(semitones);
        
        juce::String sign = (semitones >= 0) ? "+" : "";
        pitchLabel.setText("Pitch: " + sign + juce::String(semitones) + " semitones",
                          juce::dontSendNotification);
    };
    addAndMakeVisible(pitchSlider);
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
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw a simple border
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 2);
    
    // Draw placeholder for waveform
    auto waveformArea = getLocalBounds().removeFromBottom(getHeight() / 2).reduced(20);
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(waveformArea);
    
    g.setColour(juce::Colours::lightgrey);
    g.setFont(14.0f);
    g.drawText("Waveform Display (Coming Soon)", waveformArea, juce::Justification::centred, true);
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    welcomeLabel.setBounds(bounds.removeFromTop(50));
    bounds.removeFromTop(10);
    
    auto buttonRow = bounds.removeFromTop(40);
    loadFileButton.setBounds(buttonRow.removeFromLeft(150));
    buttonRow.removeFromLeft(10);
    playPauseButton.setBounds(buttonRow.removeFromLeft(100));
    
    bounds.removeFromTop(20);
    
    // Tempo controls
    tempoLabel.setBounds(bounds.removeFromTop(30));
    tempoSlider.setBounds(bounds.removeFromTop(40));
    
    bounds.removeFromTop(10);
    
    // Pitch controls
    pitchLabel.setBounds(bounds.removeFromTop(30));
    pitchSlider.setBounds(bounds.removeFromTop(40));
    
    // Remaining space for waveform (handled in paint)
}