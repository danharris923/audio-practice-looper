#include "ExportDialog.h"

ExportDialog::ExportDialog()
{
    exportEngine_ = std::make_unique<ExportEngine>();
    
    // Setup export engine callbacks
    exportEngine_->onExportComplete = [this](bool success, const std::string& message) {
        onExportComplete(success, message);
    };
    exportEngine_->onProgressUpdate = [this](double progress) {
        onProgressUpdate(progress);
    };
    exportEngine_->onOperationUpdate = [this](const std::string& operation) {
        onOperationUpdate(operation);
    };
    
    setupUIComponents();
    updateProgressVisibility(false);
}

ExportDialog::~ExportDialog()
{
    if (exportEngine_ && exportEngine_->isExporting())
    {
        exportEngine_->cancelExport();
    }
}

void ExportDialog::setupUIComponents()
{
    // File selection
    fileLabel.setText("Output File:", juce::dontSendNotification);
    addAndMakeVisible(fileLabel);
    
    filePathEditor.setReadOnly(true);
    filePathEditor.setText("Select output file...");
    addAndMakeVisible(filePathEditor);
    
    browseButton.setButtonText("Browse...");
    browseButton.onClick = [this]() { onBrowseClicked(); };
    addAndMakeVisible(browseButton);
    
    // Format selection
    formatLabel.setText("Format:", juce::dontSendNotification);
    addAndMakeVisible(formatLabel);
    
    formatComboBox.addItem("WAV (Uncompressed)", 1);
    formatComboBox.addItem("MP3 (Compressed)", 2);
    formatComboBox.addItem("FLAC (Lossless)", 3);
    formatComboBox.addItem("OGG Vorbis", 4);
    formatComboBox.setSelectedId(1);
    formatComboBox.onChange = [this]() { onFormatChanged(); };
    addAndMakeVisible(formatComboBox);
    
    // Export range
    rangeGroup.setText("Export Range");
    addAndMakeVisible(rangeGroup);
    
    exportFullFileButton.setButtonText("Full File");
    exportFullFileButton.setRadioGroupId(1);
    exportFullFileButton.setToggleState(true, juce::dontSendNotification);
    exportFullFileButton.onClick = [this]() { onRangeOptionChanged(); };
    addAndMakeVisible(exportFullFileButton);
    
    exportRangeButton.setButtonText("Custom Range");
    exportRangeButton.setRadioGroupId(1);
    exportRangeButton.onClick = [this]() { onRangeOptionChanged(); };
    addAndMakeVisible(exportRangeButton);
    
    exportLoopButton.setButtonText("Current Loop");
    exportLoopButton.setRadioGroupId(1);
    exportLoopButton.onClick = [this]() { onRangeOptionChanged(); };
    addAndMakeVisible(exportLoopButton);
    
    // Processing options
    processingGroup.setText("Processing");
    addAndMakeVisible(processingGroup);
    
    applyTimeStretchButton.setButtonText("Apply Tempo Changes");
    applyTimeStretchButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(applyTimeStretchButton);
    
    applyPitchShiftButton.setButtonText("Apply Pitch Changes");
    applyPitchShiftButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(applyPitchShiftButton);
    
    applyEQButton.setButtonText("Apply EQ");
    applyEQButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(applyEQButton);
    
    // Audio settings
    audioGroup.setText("Audio Settings");
    addAndMakeVisible(audioGroup);
    
    sampleRateLabel.setText("Sample Rate:", juce::dontSendNotification);
    addAndMakeVisible(sampleRateLabel);
    
    sampleRateComboBox.addItem("44100 Hz", 1);
    sampleRateComboBox.addItem("48000 Hz", 2);
    sampleRateComboBox.addItem("96000 Hz", 3);
    sampleRateComboBox.setSelectedId(1);
    addAndMakeVisible(sampleRateComboBox);
    
    bitDepthLabel.setText("Bit Depth:", juce::dontSendNotification);
    addAndMakeVisible(bitDepthLabel);
    
    bitDepthComboBox.addItem("16 bit", 1);
    bitDepthComboBox.addItem("24 bit", 2);
    bitDepthComboBox.addItem("32 bit", 3);
    bitDepthComboBox.setSelectedId(2);
    addAndMakeVisible(bitDepthComboBox);
    
    // Progress display
    progressGroup.setText("Export Progress");
    addAndMakeVisible(progressGroup);
    
    progressBar.setPercentageDisplay(true);
    addAndMakeVisible(progressBar);
    
    operationLabel.setText("Ready to export", juce::dontSendNotification);
    addAndMakeVisible(operationLabel);
    
    cancelButton.setButtonText("Cancel");
    cancelButton.onClick = [this]() { onCancelClicked(); };
    addAndMakeVisible(cancelButton);
    
    // Action buttons
    exportButton.setButtonText("Export");
    exportButton.onClick = [this]() { onExportClicked(); };
    addAndMakeVisible(exportButton);
    
    closeButton.setButtonText("Close");
    closeButton.onClick = [this]() { onCloseClicked(); };
    addAndMakeVisible(closeButton);
}

void ExportDialog::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff404040));
    
    // Title bar
    auto titleArea = getLocalBounds().removeFromTop(30);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(titleArea);
    
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("Export Audio", titleArea, juce::Justification::centred, true);
}

void ExportDialog::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(35); // Title bar space
    bounds.reduce(10, 10);
    
    // File selection section
    auto fileSection = bounds.removeFromTop(60);
    fileLabel.setBounds(fileSection.removeFromTop(20));
    
    auto fileRow = fileSection.removeFromTop(25);
    browseButton.setBounds(fileRow.removeFromRight(80));
    fileRow.removeFromRight(5);
    filePathEditor.setBounds(fileRow);
    
    bounds.removeFromTop(10);
    
    // Format selection
    auto formatSection = bounds.removeFromTop(25);
    formatLabel.setBounds(formatSection.removeFromLeft(80));
    formatComboBox.setBounds(formatSection.removeFromLeft(200));
    
    bounds.removeFromTop(15);
    
    // Two column layout for the rest
    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() / 2 - 5);
    bounds.removeFromLeft(10); // Gap between columns
    auto rightColumn = bounds;
    
    // Left column: Range and processing
    rangeGroup.setBounds(leftColumn.removeFromTop(120));
    auto rangeArea = rangeGroup.getBounds().reduced(10, 25);
    
    exportFullFileButton.setBounds(rangeArea.removeFromTop(25));
    exportRangeButton.setBounds(rangeArea.removeFromTop(25));
    exportLoopButton.setBounds(rangeArea.removeFromTop(25));
    
    leftColumn.removeFromTop(15);
    
    processingGroup.setBounds(leftColumn.removeFromTop(100));
    auto processingArea = processingGroup.getBounds().reduced(10, 25);
    
    applyTimeStretchButton.setBounds(processingArea.removeFromTop(25));
    applyPitchShiftButton.setBounds(processingArea.removeFromTop(25));
    applyEQButton.setBounds(processingArea.removeFromTop(25));
    
    // Right column: Audio settings and progress
    audioGroup.setBounds(rightColumn.removeFromTop(100));
    auto audioArea = audioGroup.getBounds().reduced(10, 25);
    
    auto sampleRateRow = audioArea.removeFromTop(25);
    sampleRateLabel.setBounds(sampleRateRow.removeFromLeft(100));
    sampleRateComboBox.setBounds(sampleRateRow.removeFromLeft(120));
    
    auto bitDepthRow = audioArea.removeFromTop(25);
    bitDepthLabel.setBounds(bitDepthRow.removeFromLeft(100));
    bitDepthComboBox.setBounds(bitDepthRow.removeFromLeft(120));
    
    rightColumn.removeFromTop(15);
    
    progressGroup.setBounds(rightColumn.removeFromTop(120));
    auto progressArea = progressGroup.getBounds().reduced(10, 25);
    
    progressBar.setBounds(progressArea.removeFromTop(25));
    operationLabel.setBounds(progressArea.removeFromTop(25));
    cancelButton.setBounds(progressArea.removeFromTop(30));
    
    // Bottom buttons
    bounds = getLocalBounds();
    auto buttonArea = bounds.removeFromBottom(40).reduced(10, 5);
    
    closeButton.setBounds(buttonArea.removeFromRight(80));
    buttonArea.removeFromRight(10);
    exportButton.setBounds(buttonArea.removeFromRight(80));
}

void ExportDialog::showDialog()
{
    setVisible(true);
    updateUIState();
}

void ExportDialog::hideDialog()
{
    setVisible(false);
    if (onDialogClosed)
        onDialogClosed();
}

void ExportDialog::setExportRange(double startSeconds, double endSeconds)
{
    // This would update time sliders if implemented
}

void ExportDialog::setTotalDuration(double durationSeconds)
{
    totalDurationSeconds_ = durationSeconds;
    updateUIState();
}

void ExportDialog::setLoopPoints(double loopStartSeconds, double loopEndSeconds)
{
    loopStartSeconds_ = loopStartSeconds;
    loopEndSeconds_ = loopEndSeconds;
    updateUIState();
}

void ExportDialog::updateUIState()
{
    bool hasLoop = (loopEndSeconds_ > loopStartSeconds_) && (totalDurationSeconds_ > 0.0);
    exportLoopButton.setEnabled(hasLoop);
    
    updateEstimates();
}

void ExportDialog::updateEstimates()
{
    // This would calculate and display time/size estimates
}

void ExportDialog::updateProgressVisibility(bool showProgress)
{
    progressGroup.setVisible(showProgress);
    exportButton.setEnabled(!showProgress);
    cancelButton.setVisible(showProgress);
}

ExportEngine::ExportSettings ExportDialog::getExportSettings() const
{
    ExportEngine::ExportSettings settings;
    
    settings.outputFile = juce::File(filePathEditor.getText());
    
    if (exportFullFileButton.getToggleState())
    {
        settings.startTimeSeconds = 0.0;
        settings.endTimeSeconds = totalDurationSeconds_;
    }
    else if (exportLoopButton.getToggleState())
    {
        settings.startTimeSeconds = loopStartSeconds_;
        settings.endTimeSeconds = loopEndSeconds_;
        settings.exportLoopOnly = true;
    }
    
    // Sample rate
    int sampleRateId = sampleRateComboBox.getSelectedId();
    settings.sampleRate = (sampleRateId == 2) ? 48000 : (sampleRateId == 3) ? 96000 : 44100;
    
    // Bit depth
    int bitDepthId = bitDepthComboBox.getSelectedId();
    settings.bitDepth = (bitDepthId == 1) ? 16 : (bitDepthId == 2) ? 24 : 32;
    
    settings.numChannels = 2; // Always stereo for now
    
    settings.applyTimeStretching = applyTimeStretchButton.getToggleState();
    settings.applyPitchShifting = applyPitchShiftButton.getToggleState();
    settings.applyEQ = applyEQButton.getToggleState();
    
    return settings;
}

ExportEngine::ExportFormat ExportDialog::getSelectedFormat() const
{
    switch (formatComboBox.getSelectedId())
    {
        case 2: return ExportEngine::ExportFormat::MP3;
        case 3: return ExportEngine::ExportFormat::FLAC;
        case 4: return ExportEngine::ExportFormat::OGG;
        default: return ExportEngine::ExportFormat::WAV;
    }
}

void ExportDialog::onBrowseClicked()
{
    auto format = getSelectedFormat();
    auto extension = ExportEngine::getFormatExtension(format);
    
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Choose export location",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*" + extension);
    
    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting;
    
    fileChooser_->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
        auto result = fc.getResult();
        if (result != juce::File{})
        {
            filePathEditor.setText(result.getFullPathName());
        }
    });
}

void ExportDialog::onFormatChanged()
{
    updateEstimates();
}

void ExportDialog::onRangeOptionChanged()
{
    updateUIState();
}

void ExportDialog::onExportClicked()
{
    auto settings = getExportSettings();
    auto format = getSelectedFormat();
    
    if (!settings.outputFile.hasWriteAccess())
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon,
                                         "Export Error", 
                                         "Cannot write to selected file location.");
        return;
    }
    
    updateProgressVisibility(true);
    startTimer(100); // Update progress every 100ms
    
    exportEngine_->startExport(settings, format);
}

void ExportDialog::onCancelClicked()
{
    exportEngine_->cancelExport();
}

void ExportDialog::onCloseClicked()
{
    if (exportEngine_->isExporting())
    {
        exportEngine_->cancelExport();
    }
    hideDialog();
}

void ExportDialog::onExportComplete(bool success, const std::string& message)
{
    stopTimer();
    updateProgressVisibility(false);
    
    if (success)
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::InfoIcon,
                                         "Export Complete", 
                                         "Audio exported successfully!");
    }
    else
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon,
                                         "Export Error", 
                                         message);
    }
}

void ExportDialog::onProgressUpdate(double progress)
{
    progressBar.setProgress(progress);
}

void ExportDialog::onOperationUpdate(const std::string& operation)
{
    operationLabel.setText(operation, juce::dontSendNotification);
}

void ExportDialog::timerCallback()
{
    // Update progress from export engine
    if (exportEngine_->isExporting())
    {
        progressBar.setProgress(exportEngine_->getExportProgress());
        operationLabel.setText(exportEngine_->getCurrentOperation(), juce::dontSendNotification);
    }
}