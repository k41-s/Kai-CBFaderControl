/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor/PluginProcessor.h"
#include "../../UI/Views/SetupPageView/SetupPageView.h"
#include "../../UI/Views/PerformanceView/PerformanceView.h"

//==============================================================================
/**
*/
class KaiCBFaderControlAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    KaiCBFaderControlAudioProcessorEditor (KaiCBFaderControlAudioProcessor&);
    void init();
    void addScreens();
    void configResizing();
    void configSetupPageLambdas();
    void configPerformanceViewLambdas();
    ~KaiCBFaderControlAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void determineInitialState();
    void setAppropriateSize();
    void showCurrentView(bool showSetupPage);
	void updateWindowSize(int width, int height);
    juce::Point<int> getMaxConstrainedWindowSize();

    KaiCBFaderControlAudioProcessor& audioProcessor;
    SetupPageView setupPage;
	PerformanceView performanceView;

    bool showSetupPage = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KaiCBFaderControlAudioProcessorEditor)
};
