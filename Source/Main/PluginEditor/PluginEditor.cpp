/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

//==============================================================================
KaiCBFaderControlAudioProcessorEditor::KaiCBFaderControlAudioProcessorEditor (KaiCBFaderControlAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), setupPage(p), performanceView(p)
{
    addAndMakeVisible(setupPage);
	addAndMakeVisible(performanceView);

    setResizable(true, true);
    setResizeLimits(750, 400, 1600, 1200);
    setSize (900, 500);
}

KaiCBFaderControlAudioProcessorEditor::~KaiCBFaderControlAudioProcessorEditor()
{
}

//==============================================================================
void KaiCBFaderControlAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(36,36,36));
}

void KaiCBFaderControlAudioProcessorEditor::resized()
{
    bool showSetupPage = false;
    showCurrentView(showSetupPage);
}

void KaiCBFaderControlAudioProcessorEditor::showCurrentView(bool showSetupPage)
{
    if (showSetupPage)
    {
        setupPage.setBounds(getLocalBounds());
        performanceView.setBounds(0, 0, 0, 0);
    }
    else
    {
        performanceView.setBounds(getLocalBounds());
        setupPage.setBounds(0, 0, 0, 0);
    }
}
