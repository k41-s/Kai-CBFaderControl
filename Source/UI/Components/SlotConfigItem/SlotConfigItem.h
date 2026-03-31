#pragma once
#include<JuceHeader.h>

class SlotConfigItem : public juce::Component
{
public:
	void configSlotLabel(int slotNumber);
	void configNameEditor();

	SlotConfigItem(int slotNumber);
	~SlotConfigItem() override;

	void setToggleState(bool shouldBeActive, bool shouldNotify);
	bool isActive() const;
	std::function<void()> onToggleChanged;

	void paint(juce::Graphics& g) override;
	void resized() override;

	void setupAttachment(juce::AudioProcessorValueTreeState& state, int slotNum);
private:
	juce::Label slotLabel;
	juce::TextEditor customNameEditor;
	juce::ToggleButton activeToggle;

	juce::TextEditor ipAddressEditor;

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> activeAttachment;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlotConfigItem);
};

