#pragma once
#include <JuceHeader.h>
#include "../../../../Main/PluginProcessor/PluginProcessor.h"
#include "../../UIConstants.h"
#include "../../PrecisionSlider/PrecisionSlider.h"
#include "../BaseSlotItem/BaseSlotItem.h"
#include "../../../../Main/SlotIDs.h"

class PerformanceSlotItem : 
	public BaseSlotItem,
	public juce::AudioProcessorValueTreeState::Listener
{
public:
	PerformanceSlotItem(CBFaderControlAudioProcessor& p, int slotIndex);
	~PerformanceSlotItem() override;

	void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

	void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

	void parameterChanged(const juce::String& parameterID, float newValue) override;

	void paint(juce::Graphics& g) override;
	void resized() override;
	void paintOverChildren(juce::Graphics& g) override;
	void drawSoloSafeIndicator(juce::Graphics& g);

	void setMode(SlotMode mode);

	int getSelectionId() const override { return index; }
	std::function<void(bool isMute, bool newState, PerformanceSlotItem*)> onBulkToggleRequest;

protected:
	void updateNameFromValueTree() override;
	void setupSlotBounds() override;

	juce::String getVolumeParamID() const override { return SlotIDs::volume(index); }

	void refreshAllVisuals() override;

	bool isEventFromButton(juce::Component* comp) override;

private:
	void init(int slotIndex);
	void addListeners();
	void addMouseListenerToChildren();

	// Attachment configuration
	void configAttachments(int slotIndex);
	void configVolumeAttachment(int slotIndex);
	void configPanAttachment(int slotIndex);
	void configSoloAttachment();

	// Configuration functions
	void configComponents();
	void configPanSlider();
	void configMuteButton();
	void configSoloButton();

	void configLabels();
	void configGroupLabels();
	void configIndexLabel();
	void configNameLabel();

	// State update functions
	void updateSlotColour();
	void applyXPatchSlotColour(juce::String& hexColour);
	void revertSlotColourToDefault();

	void updateStereoState();
	void updateGroupState();
	void setAppropriateIndexLabelText();
	void updateSoloSafeVisuals();

	void drawSelectedSlotItem(juce::Graphics& g);
	void drawSlotItem(juce::Graphics& g);

	// Readonly state drawing functions
	void drawReadOnlyOverlay(juce::Graphics& g);
	void drawDarkWashLayer(juce::Graphics& g, const juce::Rectangle<int>& bounds);
	void drawHatchingTexture(juce::Graphics& g, const juce::Colour& hatchingColour, juce::Rectangle<int>& bounds);
	void drawReadonlySlotOutline(juce::Graphics& g, const juce::Colour& hatchingColour, const juce::Rectangle<int>& bounds);

	// Layout functions
	void injectPanControl(juce::Rectangle<int>& area);
	void setupTopArea(juce::Rectangle<int>& area, int currentWidth);
	void setupGroupLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupIndexLabel(juce::Rectangle<int>& topArea, int labelHeight);
	void setupNameLabel(juce::Rectangle<int>& topArea, int currentWidth, int labelHeight);
	void showNameLabelIfNeeded(int currentWidth);

	void setupMuteButton(juce::Rectangle<int>& topArea);
	void setupSoloButton(juce::Rectangle<int>& topArea);

	void setComponentsEnabled(bool isFullAccess);

	SlotMode currentMode = SlotMode::Disabled;

	bool isStereoMain = false;
	bool isStereoLinked = false;
	bool isXpStereo = false;

	juce::Label groupRoleLabel;
	juce::Slider panSlider;
	juce::TextButton soloButton{ UIButtonLabels::solo };

	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloAttachment;

	bool minimalListMode = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceSlotItem)
};