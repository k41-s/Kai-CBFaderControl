#pragma once
#include<JuceHeader.h>

class SlotConfigItem : public juce::Component
{
public:
	void configSlotLabel(int slotNumber);
	void configNameEditor();
	SlotConfigItem(int slotNumber);
	~SlotConfigItem() override;

	void paint(juce::Graphics& g) override;
	void resized() override;
private:
	juce::Label slotLabel;
	juce::TextEditor customNameEditor;
	juce::ToggleButton activeToggle;

	juce::TextEditor ipAddressEditor;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlotConfigItem);
};

