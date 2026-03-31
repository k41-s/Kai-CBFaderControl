/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

//==============================================================================
KaiCBFaderControlAudioProcessorEditor::KaiCBFaderControlAudioProcessorEditor (KaiCBFaderControlAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), setupPage(p)
{
    addAndMakeVisible(setupPage);

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
    setupPage.setBounds(getLocalBounds());
}
