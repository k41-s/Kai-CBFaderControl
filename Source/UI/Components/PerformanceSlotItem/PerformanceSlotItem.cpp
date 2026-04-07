#include "PerformanceSlotItem.h"

PerformanceSlotItem::PerformanceSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex)
	:processor(p), index(slotIndex)
{

    configComponents();
}

void PerformanceSlotItem::configComponents()
{
    configVolumeFader();
    configMuteButton();
    configSoloButton();
}

void PerformanceSlotItem::configVolumeFader()
{
    addAndMakeVisible(volumeFader);
    volumeFader.setSliderStyle(juce::Slider::LinearVertical);
    volumeFader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

void PerformanceSlotItem::configMuteButton()
{
    addAndMakeVisible(muteButton);
    muteButton.setName(UIComponentNames::muteButton);
    muteButton.setClickingTogglesState(true);
}

void PerformanceSlotItem::configSoloButton()
{
    addAndMakeVisible(soloButton);
    soloButton.setName(UIComponentNames::soloButton);
    soloButton.setClickingTogglesState(true);
}

void PerformanceSlotItem::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.drawRect(getLocalBounds(), 1);
}

void PerformanceSlotItem::resized()
{
    setupSlotBounds();
}

void PerformanceSlotItem::setupSlotBounds()
{
    auto area = getLocalBounds().reduced(5);
    setupTopArea(area);
    volumeFader.setBounds(area.reduced(2));
}

void PerformanceSlotItem::setupTopArea(juce::Rectangle<int>& area)
{
    auto topArea = area.removeFromTop(area.getHeight() * 0.25f);
    muteButton.setBounds(topArea.removeFromTop(35).reduced(2));
    soloButton.setBounds(topArea.removeFromTop(35).reduced(2));
}
