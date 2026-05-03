#pragma once
#include <JuceHeader.h>
#include "../../../Main/PluginProcessor/PluginProcessor.h"
#include "../UIConstants.h"
#include "../PrecisionSlider/PrecisionSlider.h"

class PerformanceSlotItem : public juce::Component, public juce::ValueTree::Listener
{
public:
	PerformanceSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex);
	~PerformanceSlotItem() override;

	void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
	void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

	void moveFader(const juce::MouseWheelDetails& wheel);

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	void setSelected(bool selected);
	bool getSelected() const { return isSelected; }
	int getIndex() const { return index; }
	void setMode(SlotMode mode);

	std::function<void(const juce::MouseEvent&, PerformanceSlotItem*)> onBackgroundMouseDown;
	std::function<void(const juce::MouseEvent&, PerformanceSlotItem*)> onBackgroundMouseDrag;
	std::function<void(const juce::MouseEvent&, PerformanceSlotItem*)> onBackgroundMouseUp;

private:
	void init(int slotIndex);
	void addMouseListenerToChildren();

	void configAttachments(int slotIndex);
	void configVolumeAttachment(int slotIndex);
	void preSeedSlider(juce::RangedAudioParameter* param);
	void configPanAttachment(int slotIndex);

	void configComponents();
	void configVolumeFader();
	void configPanSlider();
	void configMuteButton();
	void configSoloButton();

	void configLabels();
	void configIndexLabel();
	void configNameLabel();
	void configGroupLabel();
	void configValueLabel();
	void updateValueLabel();

	void updateNameFromValueTree();
	void updateStereoState();
	void updateGroupState();

	void setAppropriateIndexLabelText();

	void drawSelectedSlotItem(juce::Graphics& g);
	void drawSlotItem(juce::Graphics& g);

	void setupSlotBounds();
	void injectPanControl(juce::Rectangle<int>& area);

	void setupTopArea(juce::Rectangle<int>& area, int currentWidth);
	void setupGroupLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupIndexLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupNameLabel(juce::Rectangle<int>& topArea, int currentWidth, int labelHeight);
	void showNameLabelIfNeeded(int currentWidth);

	void setupMuteButton(juce::Rectangle<int>& topArea);
	void setupSoloButton(juce::Rectangle<int>& topArea);
	void setupBottomArea(juce::Rectangle<int>& area, int currentWidth);

	juce::Font sharedFont;

	SlotMode currentMode = SlotMode::Disabled;

	KaiCBFaderControlAudioProcessor& processor;
	int index;

	bool isSelected = false;
	bool isStereoMain = false;
	bool isStereoLinked = false;

	juce::Label nameLabel;
	juce::Label indexLabel;
	juce::Label groupLabel;
	juce::Label valueLabel;
	juce::Label unitLabel;
	PrecisionSlider volumeFader;
	juce::Slider panSlider;
	juce::TextButton muteButton{ UIButtonLabels::mute };
	juce::TextButton soloButton{ UIButtonLabels::solo };

	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;

	bool minimalListMode = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceSlotItem)
};