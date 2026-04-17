#pragma once
#include<JuceHeader.h>
#include "../../../Main/PluginProcessor/PluginProcessor.h"

class SlotConfigItem :
	public juce::Component,
	public juce::ValueTree::Listener
{
public:

	SlotConfigItem(KaiCBFaderControlAudioProcessor& p, int slotNum);
	~SlotConfigItem() override;

	void setToggleState(bool shouldBeActive, bool shouldNotify);
	bool isActive() const;
	std::function<void()> onToggleChanged;

	void paint(juce::Graphics& g) override;
	void resized() override;

	void valueTreePropertyChanged(juce::ValueTree& tree,
		const juce::Identifier& property) override;

	void setupAttachment(juce::AudioProcessorValueTreeState& state, int slotNum);
private:
	void configSlotLabel(int slotNumber);
	void configNameEditor(int slotNumber);
	void configSaveCustomName(int slotNumber);
	void configActiveToggle();

	void setNewNameIfDifferent(juce::String& newName);

	KaiCBFaderControlAudioProcessor& processor;
	int slotNumber;

	juce::Label slotLabel;
	juce::TextEditor customNameEditor;
	juce::ToggleButton activeToggle;

	juce::TextEditor ipAddressEditor;

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> activeAttachment;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlotConfigItem);
};

