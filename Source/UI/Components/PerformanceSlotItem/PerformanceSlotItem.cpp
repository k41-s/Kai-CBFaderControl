#include "PerformanceSlotItem.h"
#include "../../../Main/SlotIDs.h"
#include "../../CustomLookAndFeel/MyColours.h"

PerformanceSlotItem::PerformanceSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex)
	:processor(p), index(slotIndex)
{
    init(slotIndex);
}

void PerformanceSlotItem::init(int slotIndex)
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
    volumeFader.addMouseListener(this, false);

    volumeFader.onResolutionChanged = [this]() { updateValueLabel(); };
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
	configIndexLabel();
    configNameLabel();
    configValueLabel();
    volumeFader.onValueChange = [this]() { updateValueLabel(); };
}

void PerformanceSlotItem::configIndexLabel()
{
    addAndMakeVisible(indexLabel);
    indexLabel.setJustificationType(juce::Justification::centred);

    auto name = "Slot " + juce::String(index);
    indexLabel.setText(name, juce::dontSendNotification);
}

void PerformanceSlotItem::configNameLabel()
{
    addAndMakeVisible(nameLabel);
    nameLabel.setJustificationType(juce::Justification::centred);

    auto name = processor.apvts.state.getProperty(SlotIDs::slotName(index), "");

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
    if (std::isnan(val) || std::isinf(val))
        val = -96.0f;

    bool isFineMode = volumeFader.getProperties().getWithDefault(UIProperties::isHighRes, false);
    juce::String text = getValueText(val, isFineMode);

    valueLabel.setText(text + " dB", juce::dontSendNotification);
}

juce::String PerformanceSlotItem::getValueText(float val, bool isFineMode)
{
    juce::String text;

    if (val <= -95.5f)
    {
        text = "-inf";
    }
    else if (isFineMode)
    {
        text = juce::String(val, 2);
    }
    else
    {
        text = juce::String(juce::roundToInt(val));
    }

	return text;
}

void PerformanceSlotItem::updateNameFromValueTree()
{
    auto customName = processor.apvts.state.getProperty(SlotIDs::slotName(index), "").toString();
    nameLabel.setText(customName, juce::dontSendNotification);
    resized();
}

void PerformanceSlotItem::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (property == juce::Identifier(SlotIDs::slotName(index)))
    {
        updateNameFromValueTree();
    }
}

void PerformanceSlotItem::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (event.eventComponent == &volumeFader)
    {
        if (wheel.deltaY != 0)
        {
            float currentVal = (float)volumeFader.getValue();

            bool isFineMode = volumeFader.getProperties().getWithDefault(UIProperties::isHighRes, false);
            float step = isFineMode ? 0.25f : 1.0f;

            float increment = (wheel.deltaY > 0) ? step : -step;

            volumeFader.setValue(currentVal + increment, juce::sendNotificationSync);

            return;
        }
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
    auto topArea = area.removeFromTop(area.getHeight() * 0.25f);

    setupIndexLabel(topArea);
    setupNameLabel(topArea, currentWidth);
    setupMuteButton(topArea);
    setupSoloButton(topArea);
}

void PerformanceSlotItem::setupIndexLabel(juce::Rectangle<int>& topArea)
{
    indexLabel.setFont(sharedFont.boldened());
    indexLabel.setBounds(topArea.removeFromTop(sharedFont.getHeight() + 5));
}

void PerformanceSlotItem::setupNameLabel(juce::Rectangle<int>& topArea, int currentWidth)
{
    const int labelHeight = sharedFont.getHeight() + 5;
    auto nameLabelBounds = topArea.removeFromTop(labelHeight);

    nameLabel.setBounds(nameLabelBounds);
    nameLabel.setFont(sharedFont);
    showNameLabelIfNeeded(currentWidth);
}

void PerformanceSlotItem::showNameLabelIfNeeded(int currentWidth)
{
    bool hasCustomName = nameLabel.getText().isNotEmpty();
    int textWidth = sharedFont.getStringWidth(nameLabel.getText());
    bool fits = currentWidth >= textWidth;

    nameLabel.setVisible(hasCustomName && fits);
}

void PerformanceSlotItem::setupMuteButton(juce::Rectangle<int>& topArea)
{
    muteButton.setBounds(topArea.removeFromTop(30).reduced(2));
}

void PerformanceSlotItem::setupSoloButton(juce::Rectangle<int>& topArea)
{
    soloButton.setBounds(topArea.removeFromTop(30).reduced(2));
}

void PerformanceSlotItem::setupBottomArea(juce::Rectangle<int>& area, int currentWidth)
{
    auto bottomArea = area.removeFromBottom(25);
    valueLabel.setFont(sharedFont);
    int requiredWidth = sharedFont.getStringWidth("-88.8 dB");
    valueLabel.setVisible(currentWidth >= requiredWidth);
    valueLabel.setBounds(bottomArea.reduced(2, 0));
}