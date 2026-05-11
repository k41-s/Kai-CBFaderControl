#include "PerformanceSlotItem.h"
#include "../../../../Main/SlotIDs.h"
#include "../../../CustomLookAndFeel/MyColours.h"
#include "../../../CustomLookAndFeel/PerformanceViewLookFeel/PerformanceViewLookFeel.h"
#include "../../../../Utils/LayoutUtils/LayoutUtils.h"
#include "../../../../Utils/UIUtils/UIUtils.h"
#include "../../../../Utils/StateUtils/SlotStateHelpers.h"

PerformanceSlotItem::PerformanceSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex)
	:BaseSlotItem(p, slotIndex)
{
    init(slotIndex);
}

void PerformanceSlotItem::init(int slotIndex)
{
    processor.apvts.state.addListener(this);

    configComponents();
    configAttachments(slotIndex);
    updateValueLabel();
    updateNameFromValueTree();
    updateStereoState();
	updateGroupState();
    addMouseListenerToChildren();
}

void PerformanceSlotItem::configComponents()
{
    configBaseVolumeFader();
    configPanSlider();
    configMuteButton();
    configSoloButton();
    configLabels();
}

void PerformanceSlotItem::configPanSlider()
{
    addChildComponent(panSlider);
    panSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
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
    configBaseValueLabel();
    configGroupLabels();
}

void PerformanceSlotItem::configGroupLabels()
{  
    configBaseGroupLabels([this]() 
    {
        return SlotStateHelpers::getGroupId(processor.apvts.state, index);
    });

    addAndMakeVisible(groupRoleLabel);
    groupRoleLabel.setJustificationType(juce::Justification::centred);
}

void PerformanceSlotItem::configIndexLabel()
{
    addAndMakeVisible(indexLabel);
    indexLabel.setJustificationType(juce::Justification::centred);

    auto indexStr = juce::String(index);
    indexLabel.setText(indexStr, juce::dontSendNotification);
}

void PerformanceSlotItem::configNameLabel()
{
    addAndMakeVisible(nameLabel);
    nameLabel.setJustificationType(juce::Justification::centred);

    auto name = SlotStateHelpers::getSlotCustomName(processor.apvts.state, index);
    nameLabel.setText(name, juce::dontSendNotification);

    nameLabel.setEditable(false, true, false);

    nameLabel.onTextChange = [this]()
    {
        SlotStateHelpers::setSlotCustomName(processor.apvts.state, index, nameLabel.getText());
    };
}

void PerformanceSlotItem::configAttachments(int slotIndex)
{
    configVolumeAttachment(slotIndex);
    configPanAttachment(slotIndex);
    configBaseMuteAttachment(SlotIDs::mute(slotIndex));
    configSoloAttachment();
}

void PerformanceSlotItem::configVolumeAttachment(int slotIndex)
{
    if (auto* param = processor.apvts.getParameter(SlotIDs::volume(slotIndex)))
        preSeedSlider(param);

	configBaseVolumeAttachment(SlotIDs::volume(slotIndex));
}

void PerformanceSlotItem::configPanAttachment(int slotIndex)
{
    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, SlotIDs::pan(slotIndex), panSlider);
}

void PerformanceSlotItem::configSoloAttachment()
{
    soloAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, SlotIDs::solo(index), soloButton);
}

void PerformanceSlotItem::updateNameFromValueTree()
{
    auto customName = SlotStateHelpers::getSlotCustomName(processor.apvts.state, index);
    nameLabel.setText(customName, juce::dontSendNotification);
    resized();
}

void PerformanceSlotItem::updateStereoState()
{
    isStereoLinked = SlotStateHelpers::isStereoLinked(processor.apvts.state, index);
    isStereoMain = SlotStateHelpers::isStereoMain(processor.apvts.state, index);
    panSlider.setVisible(isStereoMain);

    setAppropriateIndexLabelText();
    resized();
}

void PerformanceSlotItem::updateGroupState()
{
    int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, index);
    GroupRole role = SlotStateHelpers::getGroupRole(processor.apvts.state, index);

    updateBaseGroupState(grpId, true);

    if (SlotStateHelpers::isValidGroup(grpId))
    {
        int colourIdx = SlotStateHelpers::getGroupColour(processor.apvts.state, grpId);
        juce::Colour groupColour = GroupColours::palette[colourIdx];

        groupRoleLabel.setColour(juce::Label::textColourId, groupColour);
        groupRoleLabel.setText(role == GroupRole::Leader ? "L" : "S", juce::dontSendNotification);
        groupRoleLabel.setVisible(true);
    }
    else
    {
        groupRoleLabel.setText("", juce::dontSendNotification);
        groupRoleLabel.setVisible(false);
    }

	resized();
    repaint();
}

void PerformanceSlotItem::setAppropriateIndexLabelText()
{
    if (isStereoMain)
    {
        int linkedIdx = SlotStateHelpers::getLinkedSlotId(processor.apvts.state, index);
        indexLabel.setText(juce::String(index) + "-" + juce::String(linkedIdx), juce::dontSendNotification);
    }
    else 
    {
        indexLabel.setText(juce::String(index), juce::dontSendNotification);
    }
}

void PerformanceSlotItem::addMouseListenerToChildren()
{
    for (auto* child : getChildren())
    {
        child->addMouseListener(this, false);
    }
}

void PerformanceSlotItem::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    juce::String propName = property.toString();

    if (propName == SlotIDs::slotName(index))
    {
        updateNameFromValueTree();
    }
    else if (propName.startsWith(SlotIdStringPrefixes::isStereoLinked) ||
        propName.startsWith(SlotIdStringPrefixes::isStereoMain) ||
        propName.startsWith(SlotIdStringPrefixes::linkedSlotId))
    {
        updateStereoState();
    }
    else if (propName.startsWith(SlotIdStringPrefixes::groupId) ||
        propName.startsWith(SlotIdStringPrefixes::groupRole) ||
        propName.startsWith(SlotIdStringPrefixes::groupColour) ||
        propName.startsWith(SlotIdStringPrefixes::vcaName)
    ) {
        updateGroupState();
    }
}

void PerformanceSlotItem::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (currentMode != SlotMode::FullAccess)
        return;

	BaseSlotItem::mouseWheelMove(event, wheel);
}

void PerformanceSlotItem::mouseDown(const juce::MouseEvent& e)
{
    if (e.originalComponent == this || e.mods.isPopupMenu() || e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isShiftDown())
    {
        if (onBackgroundMouseDown)
            onBackgroundMouseDown(e.getEventRelativeTo(this), this);
    }
}

void PerformanceSlotItem::mouseDrag(const juce::MouseEvent& e)
{
    if (e.originalComponent == this || e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isShiftDown()) 
    {
        if (onBackgroundMouseDrag)
            onBackgroundMouseDrag(e.getEventRelativeTo(this), this);
    }
}

void PerformanceSlotItem::mouseUp(const juce::MouseEvent& e)
{
    if (e.originalComponent == this || e.mods.isPopupMenu() || e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isShiftDown()) 
    {
        if (onBackgroundMouseUp)
            onBackgroundMouseUp(e.getEventRelativeTo(this), this);
    }
}

void PerformanceSlotItem::setSelected(bool selected)
{
    if (isSelected != selected)
    {
        isSelected = selected;
        repaint();
    }
}

PerformanceSlotItem::~PerformanceSlotItem()
{
    processor.apvts.state.removeListener(this);
}

void PerformanceSlotItem::paint(juce::Graphics& g)
{
    if (isSelected)
        drawSelectedSlotItem(g);
    else
        drawSlotItem(g);

    if (currentMode == SlotMode::ReadOnly)
        drawReadOnlyOverlay(g);
}

void PerformanceSlotItem::drawSelectedSlotItem(juce::Graphics& g)
{
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.fillRect(getLocalBounds());
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.drawRect(getLocalBounds(), 2);
}

void PerformanceSlotItem::drawSlotItem(juce::Graphics& g)
{
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawRect(getLocalBounds(), 1);
}

void PerformanceSlotItem::drawReadOnlyOverlay(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto hatchingColour = juce::Colours::white.withAlpha(0.25f);

    drawDarkWashLayer(g, bounds);
    drawHatchingTexture(g, hatchingColour, bounds);
    drawReadonlySlotOutline(g, hatchingColour, bounds);
}

void PerformanceSlotItem::drawDarkWashLayer(juce::Graphics& g, const juce::Rectangle<int>& bounds)
{
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRect(bounds);
}

void PerformanceSlotItem::drawHatchingTexture(juce::Graphics& g, const juce::Colour& hatchingColour, juce::Rectangle<int>& bounds)
{
    g.setColour(hatchingColour);

    int spacing = 12;
    int maxDim = juce::jmax(bounds.getWidth(), bounds.getHeight());

    for (int i = -maxDim; i < bounds.getWidth(); i += spacing)
        g.drawLine((float)i, 0.0f, (float)(i + maxDim), (float)maxDim, 1.5f);
}

void PerformanceSlotItem::drawReadonlySlotOutline(juce::Graphics& g, const juce::Colour& hatchingColour, const juce::Rectangle<int>& bounds)
{
    g.setColour(hatchingColour);
    g.drawRect(bounds, 1);
}

void PerformanceSlotItem::resized()
{
    setupSlotBounds();
}

void PerformanceSlotItem::setupSlotBounds()
{
    auto area = getLocalBounds().reduced(2);
    int currentWidth = area.getWidth();

    setupTopArea(area, currentWidth);
    injectPanControl(area);
    setupBottomArea(area, currentWidth);
    setupFaderBounds(area);
}

void PerformanceSlotItem::setupTopArea(juce::Rectangle<int>& area, int currentWidth)
{
    juce::Font maxFont(UISizeConstants::maxFontSize);
    int labelHeight = maxFont.getHeight() + 5;

    int topAreaHeight = (labelHeight + 5) * 3 + (30 + 5) * 2;
    auto topArea = area.removeFromTop(topAreaHeight);

    setupIndexLabel(topArea, labelHeight);
    setupNameLabel(topArea, currentWidth, labelHeight);
    setupMuteButton(topArea);
    setupSoloButton(topArea);
    setupGroupLabel(topArea, labelHeight);
}

void PerformanceSlotItem::setupIndexLabel(juce::Rectangle<int>& topArea, int labelHeight)
{
    topArea.removeFromTop(5);
    indexLabel.setFont(sharedFont);
    indexLabel.setBounds(topArea.removeFromTop(labelHeight));
}

void PerformanceSlotItem::setupNameLabel(juce::Rectangle<int>& topArea, int currentWidth, int labelHeight)
{
	topArea.removeFromTop(5);
    auto nameLabelBounds = topArea.removeFromTop(labelHeight);

    nameLabel.setBounds(nameLabelBounds);
    nameLabel.setFont(sharedFont.boldened());
    showNameLabelIfNeeded(currentWidth);
}

void PerformanceSlotItem::showNameLabelIfNeeded(int currentWidth)
{
    int textWidth = sharedFont.getStringWidth(nameLabel.getText());
    bool fits = currentWidth >= textWidth;

    nameLabel.setVisible(fits);
}

void PerformanceSlotItem::setupMuteButton(juce::Rectangle<int>& topArea)
{
	topArea.removeFromTop(5);
    auto btnArea = topArea.removeFromTop(30).reduced(2);

    LayoutUtils::setCenteredMaxWidthBounds(muteButton, btnArea, SlotSizeValues::targetBtnWidth);
}

void PerformanceSlotItem::setupSoloButton(juce::Rectangle<int>& topArea)
{
	topArea.removeFromTop(5);
    auto btnArea = topArea.removeFromTop(30).reduced(2);

    LayoutUtils::setCenteredMaxWidthBounds(soloButton, btnArea, SlotSizeValues::targetBtnWidth);
}

void PerformanceSlotItem::setupGroupLabel(juce::Rectangle<int>& topArea, int labelHeight)
{
    topArea.removeFromTop(5);
    auto area = topArea.removeFromTop(labelHeight);

    groupRoleLabel.setBorderSize(juce::BorderSize<int>(0));
    groupLabel.setBorderSize(juce::BorderSize<int>(0));

    groupRoleLabel.setFont(sharedFont.boldened());
    groupLabel.setFont(sharedFont);

    if (groupRoleLabel.isVisible())
    {
        int roleWidth = sharedFont.boldened().getStringWidth(groupRoleLabel.getText()) + 4;
        groupRoleLabel.setBounds(area.removeFromLeft(roleWidth));
    }

    groupLabel.setBounds(area);
}

void PerformanceSlotItem::injectPanControl(juce::Rectangle<int>& area)
{
    if (isStereoMain) 
    {
        int panHeight = juce::jmax(35, (int)(getLocalBounds().getHeight() * 0.10f));
        auto panArea = area.removeFromTop(panHeight);

        panSlider.setBounds(panArea.reduced(2));
        panSlider.setVisible(true);
    }
    else
        panSlider.setVisible(false);
}

void PerformanceSlotItem::setMode(SlotMode mode)
{
    if (currentMode == mode) return;
    currentMode = mode;

    bool isFullAccess = (mode == SlotMode::FullAccess);
    setComponentsEnabled(isFullAccess);
    setAlpha(isFullAccess ? 1.0f : 0.4f);
}

void PerformanceSlotItem::setComponentsEnabled(bool isFullAccess)
{
    volumeFader.setEnabled(isFullAccess);
    panSlider.setEnabled(isFullAccess);
    muteButton.setEnabled(isFullAccess);
    soloButton.setEnabled(isFullAccess);
}
