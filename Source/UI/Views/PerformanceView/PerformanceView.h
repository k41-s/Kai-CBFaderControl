#pragma once
#include <JuceHeader.h>
#include "../../CustomLookAndFeel/PerformanceViewLookFeel/PerformanceViewLookFeel.h"
#include "../../Components/PerformanceSlotItem/PerformanceSlotItem.h"
#include "../../../Main/PluginProcessor/PluginProcessor.h"

class PerformanceView : public juce::Component
{
public:
	PerformanceView(KaiCBFaderControlAudioProcessor& p);
	~PerformanceView();

	void paint(juce::Graphics& g) override;
	void resized() override;
private:
	void createFaderSlots();

	void setupAndFillArea();
	void layoutSlots(juce::Rectangle<int>& area);

	KaiCBFaderControlAudioProcessor& processor;
	PerformanceViewLookFeel performanceLF;
	juce::OwnedArray<PerformanceSlotItem> slots;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceView)
};