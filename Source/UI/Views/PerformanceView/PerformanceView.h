#pragma once
#include <JuceHeader.h>
#include "../../CustomLookAndFeel/PerformanceViewLookFeel/PerformanceViewLookFeel.h"
#include "../../Components/PerformanceSlotItem/PerformanceSlotItem.h"
#include "../../../Main/PluginProcessor/PluginProcessor.h"

class PerformanceView : 
	public juce::Component,
	public juce::AudioProcessorValueTreeState::Listener,
	public juce::AsyncUpdater
{
public:
	PerformanceView(KaiCBFaderControlAudioProcessor& p);
	~PerformanceView();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void handleAsyncUpdate() override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	int getIdealWidth();
	std::function<void()> onLayoutChangeRequest;
	std::function<void()> onNavigateToSetup;
private:
	void createFaderSlots();
	void configSetupButton();
	void registerIsActiveListener();
	void deregisterIsActiveListener();

	void setupAndFillArea();
	void placeSetupButton(juce::Rectangle<int>& area);
	juce::FlexBox configFlexBox();
	void checkAndAddActiveSlots(juce::FlexBox& flexBox);
	void addSlotIfActive(bool isActive, juce::FlexBox& flexBox, PerformanceSlotItem* slot);

	juce::TextButton setupButton{ "Setup" };

	KaiCBFaderControlAudioProcessor& processor;
	PerformanceViewLookFeel performanceLF;
	juce::OwnedArray<PerformanceSlotItem> slots;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceView)
};