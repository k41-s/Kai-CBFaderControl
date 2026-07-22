#include "BaseSlotItem.h"
#include "../../../../Main/SlotIDs.h"
#include "../../../../Utils/StateUtils/SlotStateHelpers.h"
#include "../../../../Utils/UIUtils/UIUtils.h"
#include "../../../CustomLookAndFeel/PerformanceViewLookFeel/PerformanceViewLookFeel.h"

BaseSlotItem::BaseSlotItem(CBFaderControlAudioProcessor& p, int slotIndex)
    : processor(p), index(slotIndex)
{
}

BaseSlotItem::~BaseSlotItem()
{
}

void BaseSlotItem::valueTreeRedirected(juce::ValueTree& treeWhoseReferenceHasChanged)
{
	refreshAllVisuals();
}

void BaseSlotItem::configBaseVolumeFader()
{
    addAndMakeVisible(volumeFader);
    volumeFader.setSliderStyle(juce::Slider::LinearVertical);
    volumeFader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeFader.addMouseListener(this, false);
    volumeFader.getProperties().set(UIProperties::isHighRes, true);

    volumeFader.onResolutionChanged = [this]() { updateValueLabel(); };
    volumeFader.onValueChange = [this]() { updateValueLabel(); };
}

void BaseSlotItem::configBaseValueLabel()
{
    UIUtils::setupValueBoxLabel(*this, valueLabel, juce::Justification::centredRight);
    UIUtils::setupValueBoxLabel(*this, unitLabel, juce::Justification::centredLeft, UIStringConstants::unitDb);

    valueLabel.setEditable(false, true, false);

    valueLabel.onTextChange = [this]()
        {
            float newValue = valueLabel.getText().getFloatValue();
            newValue = juce::jlimit(PluginConstants::volumeMin, PluginConstants::volumeMax, newValue);

            SlotStateHelpers::setParamUnnormalized(processor.apvts, getVolumeParamID(), newValue);
        };
}

void BaseSlotItem::preSeedSlider(const juce::String& paramId)
{
    SlotStateHelpers::initSliderFromParam(processor.apvts, paramId, volumeFader);
}

void BaseSlotItem::configBaseMuteAttachment(const juce::String& paramID)
{
    muteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, paramID, muteButton);
}

void BaseSlotItem::configBaseVolumeAttachment(const juce::String& paramID)
{
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, paramID, volumeFader);
}

void BaseSlotItem::updateValueLabel()
{
    float val = (float)volumeFader.getValue();
    if (std::isnan(val) || std::isinf(val))
        val = PluginConstants::volumeMin;

    bool isFineMode = volumeFader
        .getProperties()
        .getWithDefault(
            UIProperties::isHighRes, 
            UIProperties::defaultHighRes
        );
    juce::String text = UIUtils::getValueText(val, isFineMode);

    bool isInf = (val <= PluginConstants::infCutoff);
    unitLabel.setText(isInf ? "" : UIStringConstants::unitDb, juce::dontSendNotification);
    valueLabel.setText(isInf ? text : text + " ", juce::dontSendNotification);
}

void BaseSlotItem::setupBottomArea(juce::Rectangle<int>& area, int currentWidth)
{
    auto bottomArea = area.removeFromBottom(UISizeConstants::slotBottomAreaHeight);
    valueLabel.setFont(sharedFont);
    unitLabel.setFont(sharedFont);

    int unitWidth = sharedFont.getStringWidth(UIStringConstants::unitDb) + 4;
    int valueWidth = sharedFont.getStringWidth(UIStringConstants::maxWidthValueStr);
    int requiredWidth = valueWidth + unitWidth;

    bool fits = currentWidth >= requiredWidth;
    valueLabel.setVisible(fits);
    unitLabel.setVisible(fits);

    if (fits)
    {
        int centerOffset = (bottomArea.getWidth() - requiredWidth) / 2;
        int boundaryX = centerOffset + valueWidth;

        valueLabel.setBounds(bottomArea.withWidth(boundaryX));
        unitLabel.setBounds(bottomArea.withTrimmedLeft(boundaryX));
    }
}

void BaseSlotItem::setupFaderBounds(juce::Rectangle<int>& remainingArea)
{
    auto faderArea = remainingArea.reduced(2);

    int targetFaderWidth = (targetSlotWidth > 0) ? (targetSlotWidth - 4) : faderArea.getWidth();

    if (faderArea.getWidth() > targetFaderWidth)
    {
        int shrinkAmount = (faderArea.getWidth() - targetFaderWidth) / 2;
        faderArea.reduce(shrinkAmount, 0);
    }

    volumeFader.setBounds(faderArea);
}

void BaseSlotItem::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    auto pos = event.getEventRelativeTo(this).position.toInt();

    if (volumeFader.getBounds().contains(pos) && wheel.deltaY != 0)
    {
        float currentVal = (float)volumeFader.getValue();
        bool isFineMode = volumeFader.getProperties().getWithDefault(UIProperties::isHighRes, false);
        float step = isFineMode ? PluginConstants::fineRes : PluginConstants::coarseRes;
        float increment = (wheel.deltaY > 0) ? step : -step;

        volumeFader.setValue(currentVal + increment, juce::sendNotificationSync);
    }
}

void BaseSlotItem::updateTypography()
{
    if (auto* lnf = dynamic_cast<PerformanceViewLookFeel*>(&getLookAndFeel()))
    {
        float newSize = lnf->getStandardSharedFont();

        if (sharedFont.getHeight() != newSize)
        {
            sharedFont = juce::Font(newSize);
            setupSlotBounds();
            repaint();
        }
    }
}

void BaseSlotItem::setTargetSlotWidth(int width)
{
    if (targetSlotWidth != width)
    {
        targetSlotWidth = width;
        resized();
    }
}

void BaseSlotItem::setSelected(bool selected)
{
    if (isSelected != selected)
    {
        isSelected = selected;
        repaint();
    }
}

void BaseSlotItem::setEditMode(bool isEditModeActive)
{
    if (grabHandle != nullptr)
    {
		grabHandle->setVisible(isEditModeActive);
        resized();
    }
}

bool BaseSlotItem::isEventFromButton(juce::Component* comp)
{
    if (comp == nullptr) 
        return false;
    if (comp == &muteButton || muteButton.isParentOf(comp)) 
        return true;
    return false;
}

void BaseSlotItem::paintLinkIndicator(juce::Graphics& g)
{
    int mySelectionId = getSelectionId();
    int targetTrueId = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, mySelectionId);

    if (targetTrueId != 0)
    {
        bool targetIsVca = SlotStateHelpers::getCustomLinkedIsVca(processor.apvts.state, mySelectionId);
        int linkedSelectionId = targetIsVca ? targetTrueId + PluginConstants::vcaSelectionOffset : targetTrueId;
        bool isInverse = SlotStateHelpers::isLinkPolarityInverse(processor.apvts.state, mySelectionId);

        juce::Colour pairColour;
        int colourIdx = SlotStateHelpers::getLinkColourIndex(processor.apvts.state, mySelectionId);

        if (colourIdx >= 0 && colourIdx < GroupColours::numColours)
        {
            pairColour = GroupColours::palette[colourIdx];
        }
        else
        {
            int minId = juce::jmin(mySelectionId, linkedSelectionId);
            pairColour = GroupColours::palette[minId % GroupColours::numColours];
        }

        drawLinkIndicator(g, isInverse, pairColour);
    }
}

void BaseSlotItem::drawLinkIndicator(juce::Graphics& g, bool isInverse, juce::Colour pairColour)
{
    juce::Rectangle<float> iconArea = linkIndicatorArea.toFloat();
    g.setColour(pairColour);

    float fontSize = juce::jlimit(10.0f, 14.0f, sharedFont.getHeight());
    g.setFont(juce::Font(fontSize, juce::Font::bold));

    if (isInverse)
        g.drawText("<>", iconArea, juce::Justification::centred);
    else
        g.drawText("==", iconArea, juce::Justification::centred);
}

void BaseSlotItem::mouseDown(const juce::MouseEvent& e)
{
    if (isEventFromButton(e.originalComponent)) return;
    if (e.originalComponent == this || 
        e.mods.isPopupMenu() || e.mods.isCommandDown() || 
        e.mods.isCtrlDown() || e.mods.isShiftDown())
    {
        if (onBackgroundMouseDown)
            onBackgroundMouseDown(e.getEventRelativeTo(this), this);
    }
}

void BaseSlotItem::mouseDrag(const juce::MouseEvent& e)
{
    if (isEventFromButton(e.originalComponent)) return;
    if (e.originalComponent == this || e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isShiftDown())
    {
        if (onBackgroundMouseDrag)
            onBackgroundMouseDrag(e.getEventRelativeTo(this), this);
    }
}

void BaseSlotItem::mouseUp(const juce::MouseEvent& e)
{
    if (isEventFromButton(e.originalComponent)) return;
    if (e.originalComponent == this || 
        e.mods.isPopupMenu() || e.mods.isCommandDown() ||
        e.mods.isCtrlDown() || e.mods.isShiftDown())
    {
        if (onBackgroundMouseUp)
            onBackgroundMouseUp(e.getEventRelativeTo(this), this);
    }
}

void BaseSlotItem::configBaseGroupLabels(std::function<int()> getGroupIdFunc)
{
    addAndMakeVisible(groupLabel);
    groupLabel.setJustificationType(juce::Justification::centred);

    groupLabel.onTextChange = [this, getGroupIdFunc]()
    {
        int grpId = getGroupIdFunc();
        if (grpId > 0) 
        {
            SlotStateHelpers::setVcaName(processor.apvts.state, grpId, groupLabel.getText(), &processor.undoManager);
        }
    };
}

void BaseSlotItem::updateBaseGroupState(int grpId, bool applyFaderIndicator)
{
    if (SlotStateHelpers::isValidGroup(grpId))
    {
        int colourIdx = SlotStateHelpers::getGroupColour(processor.apvts.state, grpId);
        juce::Colour groupColour = GroupColours::palette[colourIdx];

        groupLabel.setColour(juce::Label::textColourId, groupColour);
        
        if (applyFaderIndicator)
            volumeFader.getProperties().set(UIProperties::indicatorColour, groupColour.toString());

        juce::String customVcaName = SlotStateHelpers::getVcaName(processor.apvts.state, grpId);

        if (customVcaName.isNotEmpty())
        {
            groupLabel.setText(customVcaName, juce::dontSendNotification);
        }
        else
        {
            groupLabel.setText(UIGroupLabelPrefixes::group + juce::String(grpId), juce::dontSendNotification);
        }

        groupLabel.setEditable(false, true, false);
    }
    else
    {
        groupLabel.setText("", juce::dontSendNotification);
        groupLabel.setEditable(false, false, false);
        
        if (applyFaderIndicator)
            volumeFader.getProperties().remove(UIProperties::indicatorColour);
    }

    volumeFader.repaint();
    groupLabel.setVisible(true);
}

void BaseSlotItem::configGrabHandle()
{
    grabHandle = std::make_unique<GrabHandle>(getSelectionId());
    addChildComponent(grabHandle.get());
}

void BaseSlotItem::injectGrabHandle(juce::Rectangle<int>& area, int slotWidth)
{
    if (grabHandle != nullptr)
    {
        grabHandle->setBounds(0, 0, getWidth(), area.getBottom());
        grabHandle->toFront(false);
    }
}