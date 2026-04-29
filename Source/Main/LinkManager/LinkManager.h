#pragma once
#include <JuceHeader.h>

class KaiCBFaderControlAudioProcessor;

class LinkManager : public juce::AudioProcessorValueTreeState::Listener
{
public:
    LinkManager(KaiCBFaderControlAudioProcessor& processor);
    ~LinkManager() override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    KaiCBFaderControlAudioProcessor& processor;

    std::array<float, 32> lastVolume;

    std::atomic<bool> isUpdating{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LinkManager)
};