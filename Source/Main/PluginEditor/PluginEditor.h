#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor/PluginProcessor.h"
#include "../../UI/Views/SetupPageView/SetupPageView.h"
#include "../../UI/Views/PerformanceView/PerformanceView.h"

//==============================================================================

class CBFaderControlAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CBFaderControlAudioProcessorEditor (CBFaderControlAudioProcessor&);
    void init();
    void addScreens();
    void configResizing();
    void configSetupPageLambdas();
    void configPerformanceViewLambdas();
    ~CBFaderControlAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void userTriedToCloseWindow() override;

    bool keyPressed(const juce::KeyPress& key) override;

private:
    void determineInitialState();
    void setAppropriateSize();
    void showCurrentView(bool showSetupPage);
	void updateWindowSize(int width, int height);
    juce::Point<int> getMaxConstrainedWindowSize();

    CBFaderControlAudioProcessor& audioProcessor;
    SetupPageView setupPage;
	PerformanceView performanceView;

    bool showSetupPage = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CBFaderControlAudioProcessorEditor)
};
