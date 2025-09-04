#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <memory>

#include "ExportEngine.h"

class ExportDialog : public juce::Component,
                    private juce::Timer
{
public:
    ExportDialog();
    ~ExportDialog() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Show/hide dialog
    void showDialog();
    void hideDialog();
    
    // Set export range and options from current playback state
    void setExportRange(double startSeconds, double endSeconds);
    void setCurrentPosition(double positionSeconds);
    void setTotalDuration(double durationSeconds);
    void setLoopPoints(double loopStartSeconds, double loopEndSeconds);
    void setProcessingSettings(bool timeStretch, bool pitchShift, bool eq);
    
    // Callbacks
    std::function<void()> onDialogClosed;
    std::function<juce::File()> onGetSourceFile; // Get currently loaded audio file

private:
    // UI Components
    
    // File selection
    juce::Label fileLabel;
    juce::TextEditor filePathEditor;
    juce::TextButton browseButton;
    
    // Format selection
    juce::Label formatLabel;
    juce::ComboBox formatComboBox;
    
    // Export range
    juce::GroupComponent rangeGroup;
    juce::ToggleButton exportFullFileButton;
    juce::ToggleButton exportRangeButton;
    juce::ToggleButton exportLoopButton;
    
    juce::Label startTimeLabel;
    juce::Label endTimeLabel;
    juce::Slider startTimeSlider;
    juce::Slider endTimeSlider;
    
    // Loop options
    juce::Label loopRepetitionsLabel;
    juce::Slider loopRepetitionsSlider;
    
    // Audio settings
    juce::GroupComponent audioGroup;
    juce::Label sampleRateLabel;
    juce::ComboBox sampleRateComboBox;
    juce::Label bitDepthLabel;
    juce::ComboBox bitDepthComboBox;
    juce::Label channelsLabel;
    juce::ComboBox channelsComboBox;
    
    // Processing options
    juce::GroupComponent processingGroup;
    juce::ToggleButton applyTimeStretchButton;
    juce::ToggleButton applyPitchShiftButton;
    juce::ToggleButton applyEQButton;
    
    // Quality/compression
    juce::Label qualityLabel;
    juce::Slider qualitySlider;
    
    // Fade options
    juce::GroupComponent fadeGroup;
    juce::ToggleButton fadeInButton;
    juce::ToggleButton fadeOutButton;
    juce::Label fadeInDurationLabel;
    juce::Label fadeOutDurationLabel;
    juce::Slider fadeInDurationSlider;
    juce::Slider fadeOutDurationSlider;
    
    // Progress display (shown during export)
    juce::GroupComponent progressGroup;
    juce::ProgressBar progressBar;
    juce::Label operationLabel;
    juce::Label estimatesLabel;
    juce::TextButton cancelButton;
    
    // Action buttons
    juce::TextButton exportButton;
    juce::TextButton closeButton;
    
    // State
    double totalDurationSeconds_ = 0.0;
    double currentPositionSeconds_ = 0.0;
    double loopStartSeconds_ = 0.0;
    double loopEndSeconds_ = 0.0;
    bool hasTimeStretch_ = false;
    bool hasPitchShift_ = false;
    bool hasEQ_ = false;
    
    std::unique_ptr<ExportEngine> exportEngine_;
    std::unique_ptr<juce::FileChooser> fileChooser_;
    
    // Helper methods
    void setupUIComponents();
    void updateUIState();
    void updateEstimates();
    void updateProgressVisibility(bool showProgress);
    
    ExportEngine::ExportSettings getExportSettings() const;
    ExportEngine::ExportFormat getSelectedFormat() const;
    
    // Button callbacks
    void onBrowseClicked();
    void onFormatChanged();
    void onRangeOptionChanged();
    void onExportClicked();
    void onCancelClicked();
    void onCloseClicked();
    
    // Export callbacks
    void onExportComplete(bool success, const std::string& message);
    void onProgressUpdate(double progress);
    void onOperationUpdate(const std::string& operation);
    
    // Timer callback for UI updates
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExportDialog)
};