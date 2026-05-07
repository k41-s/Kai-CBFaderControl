#include "VcaSlotItem.h"
#include "../../../Main/SlotIDs.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../CustomLookAndFeel/PerformanceViewLookFeel/PerformanceViewLookFeel.h"
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
    updateColours();
}

void VcaSlotItem::configVolumeFader()
{
    addAndMakeVisible(volumeFader);
    volumeFader.setSliderStyle(juce::Slider::LinearVertical);
    volumeFader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

	volumeFader.addMouseListener(this, false);
    volumeFader.getProperties().set(UIProperties::isHighRes, true);
    volumeFader.onResolutionChanged = [this]() { updateValueLabel(); };
    volumeFader.onValueChange = [this]() { updateValueLabel(); };
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

void VcaSlotItem::updateColours()
{
    int colourIdx = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupColour(index)), 0);
    juce::Colour groupColour = GroupColours::palette[colourIdx];

    volumeFader.getProperties().set(UIProperties::customColour, groupColour.toString());
    volumeFader.repaint();

    repaint();
}

void VcaSlotItem::preSeedSlider(juce::RangedAudioParameter* param)
{
    volumeFader.setRange(param->getNormalisableRange().start, param->getNormalisableRange().end, param->getNormalisableRange().interval);
    volumeFader.setValue(param->convertFrom0to1(param->getValue()), juce::dontSendNotification);
}

void VcaSlotItem::configAttachments()
{
    configVolumeAttachment();
    configMuteAttachment();
    configExpandAttachment();
}

void VcaSlotItem::configVolumeAttachment()
{
    if (auto* param = processor.apvts.getParameter(SlotIDs::vcaVolume(index)))
        preSeedSlider(param);

    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, SlotIDs::vcaVolume(index), volumeFader);
}

void VcaSlotItem::configMuteAttachment()
{
    muteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, SlotIDs::vcaMute(index), muteButton);
}

void VcaSlotItem::configExpandAttachment()
{
    expandAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, SlotIDs::isVcaExpanded(index), expandButton);
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
	
    bool isInf = (val <= -95.75f);
    unitLabel.setText(isInf ? "" : "dB", juce::dontSendNotification);
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
    else if (property == juce::Identifier(SlotIDs::groupColour(index)))
        updateColours();
}

void VcaSlotItem::paint(juce::Graphics& g)
{
    int colourIdx = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupColour(index)), 0);
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

void VcaSlotItem::setupBottomArea(juce::Rectangle<int>& area, int currentWidth)
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

void VcaSlotItem::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    auto pos = event.getEventRelativeTo(this).position.toInt();

    if (volumeFader.getBounds().contains(pos))
    {
        if (wheel.deltaY != 0)
        {
            float currentVal = (float)volumeFader.getValue();

            bool isFineMode = volumeFader.getProperties().getWithDefault(UIProperties::isHighRes, UIProperties::defaultHighRes);
            float step = isFineMode ? 0.25f : 1.0f;

            float increment = (wheel.deltaY > 0) ? step : -step;

            volumeFader.setValue(currentVal + increment, juce::sendNotificationSync);
        }
    }
}

void VcaSlotItem::updateTypography()
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
