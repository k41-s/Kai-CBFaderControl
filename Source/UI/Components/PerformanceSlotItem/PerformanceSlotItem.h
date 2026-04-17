#pragma once
#include <JuceHeader.h>
#include "../../../Main/PluginProcessor/PluginProcessor.h"
#include "../UIConstants.h"

class PerformanceSlotItem : public juce::Component, public juce::ValueTree::Listener
{
public:
	PerformanceSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex);
	~PerformanceSlotItem() override;

	void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

	void paint(juce::Graphics& g) override;
	void resized() override;
private:
	void init(int slotIndex);
	void configComponents();
	void configVolumeFader();
	void configMuteButton();
	void configSoloButton();
	void configLabels();
	void configIndexLabel();
	void configNameLabel();
	void configValueLabel();
	void updateValueLabel();
	
	void updateNameFromValueTree();

	void setupSlotBounds();

	void setupTopArea(juce::Rectangle<int>& area, int currentWidth);
	void setupIndexLabel(juce::Rectangle<int>& topArea);
	void setupNameLabel(juce::Rectangle<int>& topArea, int currentWidth);
	void showNameLabelIfNeeded(int currentWidth);
	void setupMuteButton(juce::Rectangle<int>& topArea);
	void setupSoloButton(juce::Rectangle<int>& topArea);

	void setupBottomArea(juce::Rectangle<int>& area, int currentWidth);

	juce::Font sharedFont;

	KaiCBFaderControlAudioProcessor& processor;
	int index;

	juce::Label nameLabel;
	juce::Label indexLabel;
	juce::Label valueLabel;
	juce::Slider volumeFader;
	juce::TextButton muteButton{ UIButtonLabels::mute };
	juce::TextButton soloButton{ UIButtonLabels::solo };

	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;

	bool minimalListMode = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceSlotItem)
};