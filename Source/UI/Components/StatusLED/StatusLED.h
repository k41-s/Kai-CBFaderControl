#pragma once
#include <JuceHeader.h>

class StatusLED : public juce::Component
{
public:
    StatusLED();
    ~StatusLED() override;

    void setConnected(bool connected);

    void paint(juce::Graphics& g) override;

private:
    bool isConnected = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatusLED)
};