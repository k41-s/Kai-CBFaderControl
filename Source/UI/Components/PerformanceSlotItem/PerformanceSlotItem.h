#pragma once
#include <JuceHeader.h>
#include "../../../Main/PluginProcessor/PluginProcessor.h"
#include "../UIConstants.h"

class PerformanceSlotItem : public juce::Component
{
public:
	PerformanceSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex);

	void paint(juce::Graphics& g) override;
	void resized() override;
private:
	void configComponents();
	void configVolumeFader();
	void configMuteButton();
	void configSoloButton();

	void setupSlotBounds();
	void setupTopArea(juce::Rectangle<int>& area);

	KaiCBFaderControlAudioProcessor& processor;
	int index;

	juce::Slider volumeFader;
	juce::TextButton muteButton{ UIButtonLabels::mute };
	juce::TextButton soloButton{ UIButtonLabels::solo };

	bool minimalListMode = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceSlotItem)
};