#include "DeviceSelector.h"

DeviceSelector::DeviceSelector() = default;
DeviceSelector::~DeviceSelector() = default;

void DeviceSelector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::lightblue);
    g.setColour(juce::Colours::black);
    g.setFont(14.0f);
    g.drawText("Device Selector", getLocalBounds(), juce::Justification::centred, true);
}

void DeviceSelector::resized() {}