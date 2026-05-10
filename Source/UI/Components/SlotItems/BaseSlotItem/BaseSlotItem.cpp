#include "BaseSlotItem.h"
#include "../../../../Main/SlotIDs.h"
#include "../../../../Utils/StateUtils/SlotStateHelpers.h"
#include "../../../../Utils/UIUtils/UIUtils.h"
#include "../../../CustomLookAndFeel/PerformanceViewLookFeel/PerformanceViewLookFeel.h"

BaseSlotItem::BaseSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex)
    : processor(p), index(slotIndex)
{
}

BaseSlotItem::~BaseSlotItem()
{
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
    UIUtils::setupValueBoxLabel(*this, unitLabel, juce::Justification::centredLeft, "dB");

    valueLabel.setEditable(false, true, false);

    valueLabel.onTextChange = [this]()
        {
            float newValue = valueLabel.getText().getFloatValue();

            newValue = juce::jlimit(-96.0f, 22.0f, newValue);

            SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::volume(index), newValue);
        };
}

void BaseSlotItem::preSeedSlider(juce::RangedAudioParameter* param)
{
    volumeFader.setRange(param->getNormalisableRange().start, param->getNormalisableRange().end, param->getNormalisableRange().interval);
    volumeFader.setValue(param->convertFrom0to1(param->getValue()), juce::dontSendNotification);
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
        val = -96.0f;

    bool isFineMode = volumeFader
        .getProperties()
        .getWithDefault(
            UIProperties::isHighRes, 
            UIProperties::defaultHighRes
        );
    juce::String text = UIUtils::getValueText(val, isFineMode);

    bool isInf = (val <= -95.75f);
    unitLabel.setText(isInf ? "" : "dB", juce::dontSendNotification);
    valueLabel.setText(isInf ? text : text + " ", juce::dontSendNotification);
}

void BaseSlotItem::setupBottomArea(juce::Rectangle<int>& area, int currentWidth)
{
    auto bottomArea = area.removeFromBottom(25);
    valueLabel.setFont(sharedFont);
    unitLabel.setFont(sharedFont);

    int unitWidth = sharedFont.getStringWidth("dB") + 4;
    int valueWidth = sharedFont.getStringWidth("-88.8 ");
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
        float step = isFineMode ? 0.25f : 1.0f;
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
