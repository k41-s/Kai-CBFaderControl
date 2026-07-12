#pragma once
#include <JuceHeader.h>
#include "../BaseSlotItem/BaseSlotItem.h"
#include "../../../../Main/PluginProcessor/PluginProcessor.h"
#include "../../PrecisionSlider/PrecisionSlider.h"
#include "../../../../Main/SlotIDs.h"

class VcaSlotItem : public BaseSlotItem
{
public:
	VcaSlotItem(KaiCBFaderControlAudioProcessor& p, int vcaIndex);
	~VcaSlotItem() override;

	void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
	void paint(juce::Graphics& g) override;
	void resized() override;
	void paintOverChildren(juce::Graphics& g) override;

	int getSelectionId() const override { return index + PluginConstants::vcaSelectionOffset; }

protected:
	void updateNameFromValueTree() override;

	void refreshAllVisuals() override;

	void setupSlotBounds() override;

	bool isEventFromButton(juce::Component* comp) override;

private:
	void init();

	// Configuration functions
	void configButtons();
	void configMuteBtn();
	void configExpandBtn();
	void configLabels();
	void configIndexLabel();
	void updateColours();

	// Attachments
	void configAttachments();
	void configVolumeAttachment();
	void configExpandAttachment();

	// Layout functions
	void setupTopArea(juce::Rectangle<int>& area);
	void setupIndexLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupGroupLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupMuteButton(juce::Rectangle<int>& topArea);
	void setupExpandButton(juce::Rectangle<int>& topArea);

	bool hasAssignedMembers() const;

	juce::String getVolumeParamID() const override { return SlotIDs::vcaVolume(index); }

	juce::TextButton expandButton{ UIButtonLabels::expand };

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> expandAttachment;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VcaSlotItem)
};