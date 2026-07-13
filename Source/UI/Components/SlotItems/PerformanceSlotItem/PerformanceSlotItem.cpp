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

void PerformanceSlotItem::refreshAllVisuals()
{
    updateNameFromValueTree();
    updateSlotColour();
    updateStereoState();
    updateGroupState();
    updateSoloSafeVisuals();
}

void PerformanceSlotItem::init(int slotIndex)
{
    configGrabHandle();
    addListeners();
    configComponents();
    configAttachments(slotIndex);
    updateValueLabel();
    updateNameFromValueTree();
	updateSlotColour();
    updateStereoState();
	updateGroupState();
	updateSoloSafeVisuals();
    addMouseListenerToChildren();
}

void PerformanceSlotItem::addListeners()
{
    processor.apvts.state.addListener(this);
    processor.apvts.addParameterListener(SlotIDs::soloSafe(index), this);
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

    muteButton.onClick = [this]() 
        {
            auto mods = juce::ModifierKeys::getCurrentModifiers();
            if (mods.isShiftDown() && mods.isAltDown() && onBulkToggleRequest)
            {
                onBulkToggleRequest(true, muteButton.getToggleState(), this);
            }
        };
}

void PerformanceSlotItem::configSoloButton()
{
    addAndMakeVisible(soloButton);
    soloButton.setName(UIComponentNames::soloButton);
    soloButton.setClickingTogglesState(true);

    soloButton.onClick = [this]() 
        {
            auto mods = juce::ModifierKeys::getCurrentModifiers();
            if (mods.isShiftDown() && mods.isAltDown() && onBulkToggleRequest)
            {
                onBulkToggleRequest(false, soloButton.getToggleState(), this);
            }
        };
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
        SlotStateHelpers::setSlotCustomName(processor.apvts.state, index, nameLabel.getText(), &processor.undoManager);
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
    preSeedSlider(SlotIDs::volume(slotIndex));
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

void PerformanceSlotItem::updateSlotColour()
{
    juce::String hexColour = SlotStateHelpers::getSlotColour(processor.apvts.state, index);

    if (hexColour.isNotEmpty())
    {
        applyXPatchSlotColour(hexColour);
    }
    else
    {
        revertSlotColourToDefault();
    }

    repaint();
}

void PerformanceSlotItem::applyXPatchSlotColour(juce::String& hexColour)
{
    juce::Colour xpColour = juce::Colour::fromString(hexColour).withAlpha(1.0f);
    nameLabel.setColour(juce::Label::textColourId, xpColour);
    nameLabel.setColour(juce::Label::outlineColourId, xpColour.withAlpha(0.6f));
}

void PerformanceSlotItem::revertSlotColourToDefault()
{
    nameLabel.setColour(juce::Label::textColourId, MyColours::white);
    nameLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
}

void PerformanceSlotItem::updateStereoState()
{
    isStereoLinked = SlotStateHelpers::isStereoLinked(processor.apvts.state, index);
    isStereoMain = SlotStateHelpers::isStereoMain(processor.apvts.state, index);
    isXpStereo = SlotStateHelpers::isXpStereo(processor.apvts.state, index);

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

        if (role == GroupRole::Leader)
        {
            groupRoleLabel.setText("L", juce::dontSendNotification);
            groupRoleLabel.setColour(juce::Label::textColourId, groupColour);
            groupRoleLabel.setVisible(true);
        }
        else
        {
            groupRoleLabel.setText("", juce::dontSendNotification);
            groupRoleLabel.setVisible(false);
        }

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
        indexLabel.setText(juce::String(index) + "   -   " + juce::String(linkedIdx), juce::dontSendNotification);
    }
    else if (isXpStereo)
    {
        indexLabel.setText(juce::String(index) + " (ST)", juce::dontSendNotification);
    }
    else 
    {
        indexLabel.setText(juce::String(index), juce::dontSendNotification);
    }
}

void PerformanceSlotItem::updateSoloSafeVisuals()
{
    repaint();
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
    else if (propName == SlotIDs::slotColour(index))
    {
        updateSlotColour();
    }
    else if (SlotStateHelpers::isStereoProperty(propName))
    {
        updateStereoState();
    }
    else if (SlotStateHelpers::isGroupProperty(propName))
    {
        updateGroupState();
    }
    else if (propName.startsWith(SlotIdStringPrefixes::customLinkedId) ||
        propName.startsWith(SlotIdStringPrefixes::linkPolarityInverse))
    {
        repaint();
    }
}

void PerformanceSlotItem::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (currentMode != SlotMode::FullAccess)
        return;

	BaseSlotItem::mouseWheelMove(event, wheel);
}

bool PerformanceSlotItem::isEventFromButton(juce::Component* comp)
{
    if (BaseSlotItem::isEventFromButton(comp)) 
        return true;
    if (comp == &soloButton || soloButton.isParentOf(comp))
        return true;

    return false;
}

PerformanceSlotItem::~PerformanceSlotItem()
{
    processor.apvts.state.removeListener(this);
    processor.apvts.removeParameterListener(SlotIDs::soloSafe(index), this);
}

void PerformanceSlotItem::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == SlotIDs::soloSafe(index))
    {
        juce::MessageManager::callAsync([this]()
        {
            repaint();
        });
    }
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

void PerformanceSlotItem::paintOverChildren(juce::Graphics& g)
{
    if (SlotStateHelpers::isSlotSoloSafe(processor.apvts, index) && soloButton.isVisible())
    {
        drawSoloSafeIndicator(g);
    }

	paintLinkIndicator(g);
}

void PerformanceSlotItem::drawSoloSafeIndicator(juce::Graphics& g)
{
    auto bounds = soloButton.getBounds().reduced(3).toFloat();
    g.setColour(juce::Colours::red.withAlpha(0.3f));
    float thickness = 2.0f;
    g.drawLine(bounds.getBottomLeft().x,
        bounds.getBottomLeft().y,
        bounds.getTopRight().x,
        bounds.getTopRight().y,
        thickness);
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
    int labelHeight = maxFont.getHeight() + UISizeConstants::slotPadding;

    int indicatorHeight = 12;

    int topAreaHeight = (labelHeight + UISizeConstants::slotPadding) * 3
        + (UISizeConstants::slotBtnHeight + UISizeConstants::slotPadding) * 2
        + indicatorHeight;
    
    auto topArea = area.removeFromTop(topAreaHeight);

    setupIndexLabel(topArea, labelHeight);

    linkIndicatorArea = topArea.removeFromTop(indicatorHeight);
    injectGrabHandle(linkIndicatorArea, currentWidth);

    setupNameLabel(topArea, currentWidth, labelHeight);
    setupMuteButton(topArea);
    setupSoloButton(topArea);
    setupGroupLabel(topArea, labelHeight);
}

void PerformanceSlotItem::setupIndexLabel(juce::Rectangle<int>& topArea, int labelHeight)
{
    topArea.removeFromTop(UISizeConstants::slotPadding);
    indexLabel.setFont(sharedFont);
    indexLabel.setBounds(topArea.removeFromTop(labelHeight));
}

void PerformanceSlotItem::setupNameLabel(juce::Rectangle<int>& topArea, int currentWidth, int labelHeight)
{
	topArea.removeFromTop(UISizeConstants::slotPadding);
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
	topArea.removeFromTop(UISizeConstants::slotPadding);
    auto btnArea = topArea.removeFromTop(UISizeConstants::slotBtnHeight).reduced(2);

    LayoutUtils::setCenteredMaxWidthBounds(muteButton, btnArea, SlotSizeValues::targetBtnWidth);
}

void PerformanceSlotItem::setupSoloButton(juce::Rectangle<int>& topArea)
{
	topArea.removeFromTop(UISizeConstants::slotPadding);
    auto btnArea = topArea.removeFromTop(UISizeConstants::slotBtnHeight).reduced(2);

    LayoutUtils::setCenteredMaxWidthBounds(soloButton, btnArea, SlotSizeValues::targetBtnWidth);
}

void PerformanceSlotItem::setupGroupLabel(juce::Rectangle<int>& topArea, int labelHeight)
{
    topArea.removeFromTop(UISizeConstants::slotPadding);
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
        int panHeight = juce::jmax(UISizeConstants::minPanHeight, 
            (int)(getLocalBounds().getHeight() * UILayoutPercentages::panHeightPct));
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
