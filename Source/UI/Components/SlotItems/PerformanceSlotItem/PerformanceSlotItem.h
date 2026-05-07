#pragma once
#include <JuceHeader.h>
#include "../../../../Main/PluginProcessor/PluginProcessor.h"
#include "../../UIConstants.h"
#include "../../PrecisionSlider/PrecisionSlider.h"
#include "../BaseSlotItem/BaseSlotItem.h"

class PerformanceSlotItem : public BaseSlotItem
{
public:
	PerformanceSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex);
	~PerformanceSlotItem() override;

	void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

	void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	void setSelected(bool selected);
	bool getSelected() const { return isSelected; }
	void setMode(SlotMode mode);

	std::function<void(const juce::MouseEvent&, PerformanceSlotItem*)> onBackgroundMouseDown;
	std::function<void(const juce::MouseEvent&, PerformanceSlotItem*)> onBackgroundMouseDrag;
	std::function<void(const juce::MouseEvent&, PerformanceSlotItem*)> onBackgroundMouseUp;

protected:
	void updateNameFromValueTree() override;
	void setupSlotBounds() override;

private:
	void init(int slotIndex);
	void addMouseListenerToChildren();

	void configAttachments(int slotIndex);
	void configVolumeAttachment(int slotIndex);
	void configPanAttachment(int slotIndex);
	void configSoloAttachment();

	void configComponents();
	void configPanSlider();
	void configMuteButton();
	void configSoloButton();

	void configLabels();
	void configIndexLabel();
	void configNameLabel();
	void configGroupLabel();

	void updateStereoState();
	void updateGroupState();
	void setAppropriateIndexLabelText();

	void drawSelectedSlotItem(juce::Graphics& g);
	void drawSlotItem(juce::Graphics& g);

	void drawReadOnlyOverlay(juce::Graphics& g);
	void drawDarkWashLayer(juce::Graphics& g, const juce::Rectangle<int>& bounds);
	void drawHatchingTexture(juce::Graphics& g, const juce::Colour& hatchingColour, juce::Rectangle<int>& bounds);
	void drawReadonlySlotOutline(juce::Graphics& g, const juce::Colour& hatchingColour, const juce::Rectangle<int>& bounds);

	void injectPanControl(juce::Rectangle<int>& area);
	void setupTopArea(juce::Rectangle<int>& area, int currentWidth);
	void setupGroupLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupIndexLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupNameLabel(juce::Rectangle<int>& topArea, int currentWidth, int labelHeight);
	void showNameLabelIfNeeded(int currentWidth);

	void setupMuteButton(juce::Rectangle<int>& topArea);
	void setupSoloButton(juce::Rectangle<int>& topArea);

	SlotMode currentMode = SlotMode::Disabled;

	bool isSelected = false;
	bool isStereoMain = false;
	bool isStereoLinked = false;

	juce::Label groupLabel;
	juce::Slider panSlider;
	juce::TextButton soloButton{ UIButtonLabels::solo };

	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloAttachment;

	bool minimalListMode = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceSlotItem)
};