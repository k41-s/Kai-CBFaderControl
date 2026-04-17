#include "PerformanceSlotItem.h"
#include "../../../Main/SlotIDs.h"
#include "../../CustomLookAndFeel/MyColours.h"

PerformanceSlotItem::PerformanceSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex)
	:processor(p), index(slotIndex)
{
	processor.apvts.state.addListener(this);

    configComponents();
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, SlotIDs::volume(slotIndex), volumeFader);
    updateValueLabel();
	updateNameFromValueTree();
}

void PerformanceSlotItem::configComponents()
{
    configVolumeFader();
    configMuteButton();
    configSoloButton();
    configLabels();
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

void PerformanceSlotItem::configLabels()
{
    configNameLabel();
    configValueLabel();
    volumeFader.onValueChange = [this]() { updateValueLabel(); };
}

void PerformanceSlotItem::configNameLabel()
{
    addAndMakeVisible(nameLabel);
    nameLabel.setJustificationType(juce::Justification::centred);

    auto name = processor.apvts.state.getProperty(SlotIDs::slotName(index), "Slot " + juce::String(index));
    nameLabel.setText(name, juce::dontSendNotification);
}

void PerformanceSlotItem::configValueLabel()
{
    addAndMakeVisible(valueLabel);
    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::backgroundColourId, MyColours::unpressedBtn);
    valueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
}

void PerformanceSlotItem::updateValueLabel()
{
    float val = (float)volumeFader.getValue();
    juce::String text = (val <= -95.5f) ? "-inf" : juce::String(val, 1);
    valueLabel.setText(text + " dB", juce::dontSendNotification);
}

void PerformanceSlotItem::updateNameFromValueTree()
{
    auto name = processor.apvts.state.getProperty(SlotIDs::slotName(index), "Slot " + juce::String(index));
    nameLabel.setText(name, juce::dontSendNotification);
}

void PerformanceSlotItem::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (property == juce::Identifier(SlotIDs::slotName(index)))
    {
        updateNameFromValueTree();
    }
}

PerformanceSlotItem::~PerformanceSlotItem()
{
    processor.apvts.state.removeListener(this);
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
    auto area = getLocalBounds().reduced(2);
    int currentWidth = area.getWidth();

    float sharedFontSize = juce::jlimit(10.0f, 16.0f, (float)currentWidth * 0.25f);
	sharedFont = juce::Font(sharedFontSize);

    setupTopArea(area, currentWidth);
    setupBottomArea(area, currentWidth);
    volumeFader.setBounds(area.reduced(2));
}

void PerformanceSlotItem::setupTopArea(juce::Rectangle<int>& area, int currentWidth)
{
    auto topArea = area.removeFromTop(area.getHeight() * 0.3f);
    setupNameLabel(topArea, currentWidth);
    setupMuteButton(topArea);
    setupSoloButton(topArea);
}

void PerformanceSlotItem::setupNameLabel(juce::Rectangle<int>& topArea, int currentWidth)
{
    nameLabel.setFont(sharedFont);
    nameLabel.setBounds(topArea.removeFromTop(sharedFont.getHeight() + 2));

    int requiredNameWidth = sharedFont.getStringWidth(nameLabel.getText());
    nameLabel.setVisible(currentWidth >= requiredNameWidth);
}

void PerformanceSlotItem::setupMuteButton(juce::Rectangle<int>& topArea)
{
    muteButton.setBounds(topArea.removeFromTop(35).reduced(2));
}

void PerformanceSlotItem::setupSoloButton(juce::Rectangle<int>& topArea)
{
    soloButton.setBounds(topArea.removeFromTop(35).reduced(2));
}

void PerformanceSlotItem::setupBottomArea(juce::Rectangle<int>& area, int currentWidth)
{
    auto bottomArea = area.removeFromBottom(25);
    valueLabel.setFont(sharedFont);
    int requiredWidth = sharedFont.getStringWidth("-88.8 dB");
    valueLabel.setVisible(currentWidth >= requiredWidth);
    valueLabel.setBounds(bottomArea.reduced(2, 0));
}