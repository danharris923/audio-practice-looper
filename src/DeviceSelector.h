#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class DeviceSelector : public juce::Component
{
public:
    DeviceSelector();
    ~DeviceSelector() override;
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceSelector)
};