#include "VcaSlotItem.h"
#include "../../../../Main/SlotIDs.h"
#include "../../../CustomLookAndFeel/MyColours.h"
#include "../../../CustomLookAndFeel/PerformanceViewLookFeel/PerformanceViewLookFeel.h"
#include "../../../../Utils/LayoutUtils/LayoutUtils.h"
#include "../../../../Utils/UIUtils/UIUtils.h"
#include "../../../../Utils/StateUtils/SlotStateHelpers.h"

VcaSlotItem::VcaSlotItem(KaiCBFaderControlAudioProcessor& p, int vcaIndex)
	: BaseSlotItem(p, vcaIndex)
{
    init();
}

void VcaSlotItem::init()
{
    processor.apvts.state.addListener(this);

    configBaseVolumeFader();
    configButtons();
    configLabels();
    configAttachments();
    updateNameFromValueTree();
    updateValueLabel();
    updateColours();
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
    configBaseValueLabel();
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

void VcaSlotItem::updateColours()
{
    int colourIdx = SlotStateHelpers::getGroupColour(processor.apvts.state, index);
    juce::Colour groupColour = GroupColours::palette[colourIdx];

    volumeFader.getProperties().set(UIProperties::customColour, groupColour.toString());
    volumeFader.repaint();

    repaint();
}

void VcaSlotItem::configAttachments()
{
    configVolumeAttachment();
    configBaseMuteAttachment(SlotIDs::vcaMute(index));
    configExpandAttachment();
}

void VcaSlotItem::configVolumeAttachment()
{
    if (auto* param = processor.apvts.getParameter(SlotIDs::vcaVolume(index)))
        preSeedSlider(param);

	configBaseVolumeAttachment(SlotIDs::vcaVolume(index));
}

void VcaSlotItem::configExpandAttachment()
{
    expandAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, SlotIDs::isVcaExpanded(index), expandButton);
}

void VcaSlotItem::updateNameFromValueTree()
{
    juce::String customName = SlotStateHelpers::getVcaName(processor.apvts.state, index);
    nameLabel.setText(customName, juce::dontSendNotification);
    resized();
}

VcaSlotItem::~VcaSlotItem()
{
	processor.apvts.state.removeListener(this);
}

void VcaSlotItem::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (property == juce::Identifier(SlotIDs::vcaName(index)))
        updateNameFromValueTree();
    else if (property == juce::Identifier(SlotIDs::groupColour(index)))
        updateColours();
}

void VcaSlotItem::paint(juce::Graphics& g)
{
    int colourIdx = SlotStateHelpers::getGroupColour(processor.apvts.state, index);
    juce::Colour groupColour = GroupColours::palette[colourIdx];

    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.fillAll();

    g.setColour(groupColour.withAlpha(0.6f));
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
    
    setupTopArea(area);
    setupBottomArea(area, currentWidth);
    setupFaderBounds(area);
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
