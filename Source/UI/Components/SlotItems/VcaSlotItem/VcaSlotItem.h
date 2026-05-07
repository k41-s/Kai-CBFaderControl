#pragma once
#include <JuceHeader.h>
#include "../BaseSlotItem/BaseSlotItem.h"
#include "../../../../Main/PluginProcessor/PluginProcessor.h"
#include "../../PrecisionSlider/PrecisionSlider.h"

class VcaSlotItem : public BaseSlotItem
{
public:
	VcaSlotItem(KaiCBFaderControlAudioProcessor& p, int vcaIndex);
	~VcaSlotItem() override;

	void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
	void paint(juce::Graphics& g) override;
	void resized() override;

protected:
	void updateNameFromValueTree() override;
	void setupSlotBounds() override;

private:
	void init();
	void configButtons();
	void configMuteBtn();
	void configExpandBtn();
	void configLabels();
	void configIndexLabel();
	void configNameLabel();
	void updateColours();

	void configAttachments();
	void configVolumeAttachment();
	void configExpandAttachment();

	void setupTopArea(juce::Rectangle<int>& area);
	void setupIndexLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupNameLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupMuteButton(juce::Rectangle<int>& topArea);
	void setupExpandButton(juce::Rectangle<int>& topArea);

	juce::TextButton expandButton{ UIButtonLabels::expand };

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> expandAttachment;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VcaSlotItem)
};