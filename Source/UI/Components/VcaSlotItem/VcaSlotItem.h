#pragma once
#include <JuceHeader.h>
#include "../../../Main/PluginProcessor/PluginProcessor.h"
#include "../PrecisionSlider/PrecisionSlider.h"

class VcaSlotItem : public juce::Component, public juce::ValueTree::Listener
{
public:
	VcaSlotItem(KaiCBFaderControlAudioProcessor& p, int vcaIndex);
	~VcaSlotItem() override;

	void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
	void paint(juce::Graphics& g) override;
	void resized() override;

	int getIndex() const { return index; }

private:
	void init();
	void configVolumeFader();
	void configButtons();
	void configMuteBtn();
	void configExpandBtn();
	void configLabels();
	void configIndexLabel();
	void configNameLabel();
	void configValueLabel();
	void configAttachments();

	void updateValueLabel();
	void updateNameFromValueTree();

	void setupSlotBounds();
	void setupTopArea(juce::Rectangle<int>& area);
	void setupIndexLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupNameLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupMuteButton(juce::Rectangle<int>& topArea);
	void setupExpandButton(juce::Rectangle<int>& topArea);
	void setupBottomArea(juce::Rectangle<int>& area);

	juce::Font sharedFont;

	KaiCBFaderControlAudioProcessor& processor;
	int index;

	juce::Label nameLabel;
	juce::Label indexLabel;
	juce::Label valueLabel;
	juce::Label unitLabel;
	PrecisionSlider volumeFader;
	juce::TextButton muteButton{ "M" };
	juce::TextButton expandButton{ "EXP" };

	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> expandAttachment;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VcaSlotItem)
};