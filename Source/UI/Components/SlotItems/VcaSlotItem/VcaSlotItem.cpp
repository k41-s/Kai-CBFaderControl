#include "VcaSlotItem.h"
#include "../../../../Main/SlotIDs.h"
#include "../../../CustomLookAndFeel/MyColours.h"
#include "../../../CustomLookAndFeel/PerformanceViewLookFeel/PerformanceViewLookFeel.h"
#include "../../../../Utils/LayoutUtils/LayoutUtils.h"
#include "../../../../Utils/UIUtils/UIUtils.h"
#include "../../../../Utils/StateUtils/SlotStateHelpers.h"
#include "../../../../Utils/Enums/ContextMenuId.h"

VcaSlotItem::VcaSlotItem(KaiCBFaderControlAudioProcessor& p, int vcaIndex)
	: BaseSlotItem(p, vcaIndex)
{
    init();
}

void VcaSlotItem::init()
{
	configGrabHandle();

    processor.apvts.state.addListener(this);

    configBaseVolumeFader();
    configButtons();
    configLabels();
    configAttachments();
    updateBaseGroupState(index);
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
    configBaseValueLabel();
    configBaseGroupLabels([this]() { return index; });
}

void VcaSlotItem::configIndexLabel()
{
    indexLabel.setJustificationType(juce::Justification::centred);
    indexLabel.setText(UIGroupLabelPrefixes::vca + juce::String(index), juce::dontSendNotification);
    addAndMakeVisible(indexLabel);
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
    preSeedSlider(SlotIDs::vcaVolume(index));
    configBaseVolumeAttachment(SlotIDs::vcaVolume(index));
}

void VcaSlotItem::configExpandAttachment()
{
    expandAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, SlotIDs::isVcaExpanded(index), expandButton);
}

void VcaSlotItem::updateNameFromValueTree()
{
    // Deliberately left empty. 
    // VcaSlotItem uses the base class updateBaseGroupState() logic instead.
}

void VcaSlotItem::refreshAllVisuals()
{
    updateBaseGroupState(index);
    updateColours();
    resized();
}

VcaSlotItem::~VcaSlotItem()
{
	processor.apvts.state.removeListener(this);
}

void VcaSlotItem::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    juce::String propName = property.toString();

    if (propName == SlotIDs::vcaName(index) || propName == SlotIDs::groupColour(index))
    {
        updateBaseGroupState(index);
        updateColours();
        resized();
    }
    else if (SlotStateHelpers::isGroupProperty(propName))
    {
        expandButton.setVisible(hasAssignedMembers());
    }
    else if (propName.startsWith(SlotIdStringPrefixes::customLinkedId) || propName.startsWith(SlotIdStringPrefixes::linkPolarityInverse))
    {
        repaint();
    }
}

void VcaSlotItem::paint(juce::Graphics& g)
{
    int colourIdx = SlotStateHelpers::getGroupColour(processor.apvts.state, index);
    juce::Colour groupColour = GroupColours::palette[colourIdx];

    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.fillAll();

    g.setColour(groupColour.withAlpha(0.6f));
    g.drawRect(getLocalBounds(), 1);

    if (isSelected)
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colours::white.withAlpha(0.15f));
        g.fillRect(bounds);
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.drawRect(bounds, 2.0f);
    }
}

void VcaSlotItem::resized()
{
    setupSlotBounds();
}

void VcaSlotItem::paintOverChildren(juce::Graphics& g)
{
	paintLinkIndicator(g);
}

void VcaSlotItem::setupSlotBounds()
{
    auto area = getLocalBounds().reduced(2);
    int currentWidth = area.getWidth();
    
    setupTopArea(area, currentWidth);
    setupBottomArea(area, currentWidth);
    setupFaderBounds(area);
}

void VcaSlotItem::setupTopArea(juce::Rectangle<int>& area, int currentWidth)
{
    juce::Font maxFont(UISizeConstants::maxFontSize);
    int labelHeight = maxFont.getHeight() + UISizeConstants::slotPadding;

    int indicatorHeight = 12;

    int topAreaHeight = (labelHeight + UISizeConstants::slotPadding) * 3
        + (UISizeConstants::slotBtnHeight + UISizeConstants::slotPadding) * 2
        + indicatorHeight;
    
    auto topArea = area.removeFromTop(topAreaHeight);

    setupIndexLabel(topArea, labelHeight);

    linkIndicatorArea = topArea.removeFromTop(indicatorHeight);
    injectGrabHandle(linkIndicatorArea, currentWidth);

    setupGroupLabel(topArea, labelHeight);
    setupMuteButton(topArea);
    setupExpandButton(topArea);

    // Allocate blank space matching where groupLabel sits in regular slots
    topArea.removeFromTop(UISizeConstants::slotPadding);
    topArea.removeFromTop(labelHeight);
}

void VcaSlotItem::setupIndexLabel(juce::Rectangle<int>& topArea, int labelHeight)
{
    topArea.removeFromTop(UISizeConstants::slotPadding);
    indexLabel.setFont(sharedFont);
    indexLabel.setBounds(topArea.removeFromTop(labelHeight));
}

void VcaSlotItem::setupGroupLabel(juce::Rectangle<int>& topArea, int labelHeight)
{
    topArea.removeFromTop(UISizeConstants::slotPadding);

    groupLabel.setBorderSize(juce::BorderSize<int>(0));

    groupLabel.setFont(sharedFont.boldened());
    groupLabel.setBounds(topArea.removeFromTop(labelHeight));
}

void VcaSlotItem::setupMuteButton(juce::Rectangle<int>& topArea)
{
    topArea.removeFromTop(UISizeConstants::slotPadding);
    auto muteArea = topArea.removeFromTop(UISizeConstants::slotBtnHeight).reduced(2);
    LayoutUtils::setCenteredMaxWidthBounds(muteButton, muteArea, SlotSizeValues::targetBtnWidth);
}

void VcaSlotItem::setupExpandButton(juce::Rectangle<int>& topArea)
{
    topArea.removeFromTop(UISizeConstants::slotPadding);
    expandButton.setVisible(hasAssignedMembers());

    auto expandArea = topArea.removeFromTop(UISizeConstants::slotBtnHeight).reduced(2);
    LayoutUtils::setCenteredMaxWidthBounds(expandButton, expandArea, SlotSizeValues::targetBtnWidth);
}

bool VcaSlotItem::hasAssignedMembers() const
{
    auto& state = processor.apvts.state;

    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        if (SlotStateHelpers::getGroupId(state, i) == index)
        {
            return true;
        }
    }
    return false;
}

bool VcaSlotItem::isEventFromButton(juce::Component* comp)
{
    if (BaseSlotItem::isEventFromButton(comp)) 
        return true;

    if (comp == &expandButton || expandButton.isParentOf(comp)) 
        return true;

    return false;
}