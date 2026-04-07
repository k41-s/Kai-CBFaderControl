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
    ~KaiCBFaderControlAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void showCurrentView(bool showSetupPage);

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    KaiCBFaderControlAudioProcessor& audioProcessor;

    SetupPageView setupPage;
	PerformanceView performanceView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KaiCBFaderControlAudioProcessorEditor)
};
