#include "VcaSlotItem.h"
#include "../../../Main/SlotIDs.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../../Utils/LayoutUtils/LayoutUtils.h"
#include "../../../Utils/UIUtils/UIUtils.h"

VcaSlotItem::VcaSlotItem(KaiCBFaderControlAudioProcessor& p, int vcaIndex)
	: processor(p), index(vcaIndex)
{
    init();
}

void VcaSlotItem::init()
{
    processor.apvts.state.addListener(this);

    configVolumeFader();
    configButtons();
    configLabels();
    configAttachments();
    updateNameFromValueTree();
    updateValueLabel();
}

void VcaSlotItem::configVolumeFader()
{
    volumeFader.getProperties().set(UIProperties::customColour, juce::Colours::darkred.brighter(0.2f).toString());
    volumeFader.setSliderStyle(juce::Slider::LinearVertical);
    volumeFader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeFader.onResolutionChanged = [this]() { updateValueLabel(); };
    volumeFader.onValueChange = [this]() { updateValueLabel(); };
    addAndMakeVisible(volumeFader);
}

void VcaSlotItem::configButtons()
{
    configMuteBtn();
    configExpandBtn();
}

void VcaSlotItem::configMuteBtn()
{
	muteButton.setName(UIComponentNames::muteButton);
    muteButton.setClickingTogglesState(true);
    addAndMakeVisible(muteButton);
}

void VcaSlotItem::configExpandBtn()
{
	expandButton.setName(UIComponentNames::expandButton);
    expandButton.setClickingTogglesState(true);
    addAndMakeVisible(expandButton);
}

void VcaSlotItem::configLabels()
{
    configIndexLabel();
    configNameLabel();
    configValueLabel();
}

void VcaSlotItem::configIndexLabel()
{
    indexLabel.setJustificationType(juce::Justification::centred);
    indexLabel.setText("VCA " + juce::String(index), juce::dontSendNotification);
    addAndMakeVisible(indexLabel);
}

void VcaSlotItem::configNameLabel()
{
    nameLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(nameLabel);
}

void VcaSlotItem::configValueLabel()
{
    UIUtils::setupValueBoxLabel(*this, valueLabel, juce::Justification::centredRight);
    UIUtils::setupValueBoxLabel(*this, unitLabel, juce::Justification::centredLeft, "dB");
}

void VcaSlotItem::configAttachments()
{
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, SlotIDs::vcaVolume(index), volumeFader);
    muteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts, SlotIDs::vcaMute(index), muteButton);
    expandAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts, SlotIDs::isVcaExpanded(index), expandButton);
}

void VcaSlotItem::updateNameFromValueTree()
{
    auto customName = processor.apvts.state.getProperty(SlotIDs::vcaName(index), "").toString();
    nameLabel.setText(customName, juce::dontSendNotification);
    resized();
}

void VcaSlotItem::updateValueLabel()
{
    float val = (float)volumeFader.getValue();
    if (std::isnan(val) || std::isinf(val)) 
        val = -96.0f;

    bool isFineMode = volumeFader.getProperties().getWithDefault(UIProperties::isHighRes, UIProperties::defaultHighRes);
    juce::String text = UIUtils::getValueText(val, isFineMode);
	
    bool isInf = (text == UIStringConstants::inf);
	unitLabel.setVisible(!isInf);
    valueLabel.setText(isInf ? text : text + " ", juce::dontSendNotification);
}

VcaSlotItem::~VcaSlotItem()
{
	processor.apvts.state.removeListener(this);
}

void VcaSlotItem::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (property == juce::Identifier(SlotIDs::vcaName(index)))
        updateNameFromValueTree();
}

void VcaSlotItem::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.fillAll();
    g.setColour(juce::Colours::darkred.withAlpha(0.6f));
    g.drawRect(getLocalBounds(), 1);
}

void VcaSlotItem::resized()
{
    setupSlotBounds();
}


void VcaSlotItem::setupSlotBounds()
{
    auto area = getLocalBounds().reduced(2);
    int currentWidth = area.getWidth();
    UIUtils::setSharedFont(sharedFont, currentWidth);


    setupTopArea(area);
    setupBottomArea(area);
    volumeFader.setBounds(area.reduced(2));
}

void VcaSlotItem::setupTopArea(juce::Rectangle<int>& area)
{
    juce::Font maxFont(UISizeConstants::maxFontSize);
    int labelHeight = maxFont.getHeight() + 5;

    int topAreaHeight = (labelHeight + 5) * 3 + (30 + 5) * 2;
    auto topArea = area.removeFromTop(topAreaHeight);

    setupIndexLabel(topArea, labelHeight);
    setupNameLabel(topArea, labelHeight);
    setupMuteButton(topArea);
    setupExpandButton(topArea);

    // Allocate blank space matching where groupLabel sits in regular slots
    topArea.removeFromTop(5);
    topArea.removeFromTop(labelHeight);
}

void VcaSlotItem::setupIndexLabel(juce::Rectangle<int>& topArea, int labelHeight)
{
    topArea.removeFromTop(5);
    indexLabel.setFont(sharedFont);
    indexLabel.setBounds(topArea.removeFromTop(labelHeight));
}

void VcaSlotItem::setupNameLabel(juce::Rectangle<int>& topArea, int labelHeight)
{
    topArea.removeFromTop(5);
    nameLabel.setFont(sharedFont.boldened());
    nameLabel.setBounds(topArea.removeFromTop(labelHeight));
}

void VcaSlotItem::setupMuteButton(juce::Rectangle<int>& topArea)
{
    topArea.removeFromTop(5);
    auto muteArea = topArea.removeFromTop(30).reduced(2);
    LayoutUtils::setCenteredMaxWidthBounds(muteButton, muteArea, SlotSizeValues::targetBtnWidth);
}

void VcaSlotItem::setupExpandButton(juce::Rectangle<int>& topArea)
{
    topArea.removeFromTop(5);
    auto expandArea = topArea.removeFromTop(30).reduced(2);
    LayoutUtils::setCenteredMaxWidthBounds(expandButton, expandArea, SlotSizeValues::targetBtnWidth);
}

void VcaSlotItem::setupBottomArea(juce::Rectangle<int>& area)
{
    auto bottomArea = area.removeFromBottom(25);
    valueLabel.setFont(sharedFont);
    unitLabel.setFont(sharedFont);

    auto bounds = bottomArea.reduced(2, 0);
    int unitWidth = sharedFont.getStringWidth("dB") + 4;

    unitLabel.setBounds(bounds.removeFromRight(unitWidth));
    valueLabel.setBounds(bounds);
}
