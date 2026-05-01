#include "PerformanceSlotItem.h"
#include "../../../Main/SlotIDs.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../../Utils/LayoutUtils/LayoutUtils.h"

PerformanceSlotItem::PerformanceSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex)
	:processor(p), index(slotIndex)
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
    configVolumeFader();
    configPanSlider();
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
    configGroupLabel();
    configValueLabel();
    volumeFader.onValueChange = [this]() { updateValueLabel(); };
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

    auto name = processor.apvts.state.getProperty(SlotIDs::slotName(index), "");

    nameLabel.setText(name, juce::dontSendNotification);
}

void PerformanceSlotItem::configGroupLabel()
{
    addAndMakeVisible(groupLabel);
    groupLabel.setJustificationType(juce::Justification::centred);
	groupLabel.setColour(juce::Label::textColourId, juce::Colours::cyan.withAlpha(0.8f));
}

void PerformanceSlotItem::configValueLabel()
{
    addAndMakeVisible(valueLabel);
    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::backgroundColourId, MyColours::unpressedBtn);
    valueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
}

void PerformanceSlotItem::configAttachments(int slotIndex)
{
    configVolumeAttachment(slotIndex);
    configPanAttachment(slotIndex);
}

void PerformanceSlotItem::configVolumeAttachment(int slotIndex)
{
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, SlotIDs::volume(slotIndex), volumeFader);
}

void PerformanceSlotItem::configPanAttachment(int slotIndex)
{
    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, SlotIDs::pan(slotIndex), panSlider);
}

void PerformanceSlotItem::updateValueLabel()
{
    float val = (float)volumeFader.getValue();
    if (std::isnan(val) || std::isinf(val))
        val = -96.0f;

    bool isFineMode = volumeFader.getProperties().getWithDefault(UIProperties::isHighRes, UIProperties::defaultHighRes);
    juce::String text = getValueText(val, isFineMode);

    valueLabel.setText(text + " dB", juce::dontSendNotification);
}

juce::String PerformanceSlotItem::getValueText(float val, bool isFineMode)
{
    juce::String text;

    if (val <= -95.75f)
        text = "-inf";
    else if (isFineMode)
        text = juce::String(val, 2);
    else
        text = juce::String(juce::roundToInt(val));

	return text;
}

void PerformanceSlotItem::updateNameFromValueTree()
{
    auto customName = processor.apvts.state.getProperty(SlotIDs::slotName(index), "").toString();
    nameLabel.setText(customName, juce::dontSendNotification);
    resized();
}

void PerformanceSlotItem::updateStereoState()
{
    isStereoLinked = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::isStereoLinked(index)), false);
    isStereoMain = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::isStereoMain(index)), false);
    panSlider.setVisible(isStereoMain);

    setAppropriateIndexLabelText();
    resized();
}

void PerformanceSlotItem::updateGroupState()
{
    int grpId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(index)), 0);
    int role = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupRole(index)), 0);

    if (grpId > 0) 
    {
        juce::String labelText = (role == 1) ? "LDR " : "GRP ";
        labelText += juce::String(grpId);
        groupLabel.setText(labelText, juce::dontSendNotification);
    }
    else 
    {
        groupLabel.setText("", juce::dontSendNotification);
    }
    groupLabel.setVisible(true);
    repaint();
}

void PerformanceSlotItem::setAppropriateIndexLabelText()
{
    if (isStereoMain) {
        int linkedIdx = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::linkedSlotId(index)), index);
        indexLabel.setText(juce::String(index) + "-" + juce::String(linkedIdx), juce::dontSendNotification);
    }
    else {
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

    if (propName == SlotIDs::slotName(index)) {
        updateNameFromValueTree();
    }
    else if (propName.startsWith("isStereo") || propName.startsWith("linkedSlotId")) {
        updateStereoState();
    }
    else if (propName.startsWith("groupId") || propName.startsWith("groupRole")) {
        updateGroupState();
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

void PerformanceSlotItem::mouseDown(const juce::MouseEvent& e)
{
    if (e.originalComponent == this || e.mods.isPopupMenu() || e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isShiftDown()) {
        if (onBackgroundMouseDown)
            onBackgroundMouseDown(e.getEventRelativeTo(this), this);
    }
}

void PerformanceSlotItem::mouseDrag(const juce::MouseEvent& e)
{
    if (e.originalComponent == this || e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isShiftDown()) {
        if (onBackgroundMouseDrag)
            onBackgroundMouseDrag(e.getEventRelativeTo(this), this);
    }
}

void PerformanceSlotItem::mouseUp(const juce::MouseEvent& e)
{
    if (e.originalComponent == this || e.mods.isPopupMenu() || e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isShiftDown()) {
        if (onBackgroundMouseUp)
            onBackgroundMouseUp(e.getEventRelativeTo(this), this);
    }
}

void PerformanceSlotItem::setSelected(bool selected)
{
    if (isSelected != selected) {
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
    injectPanControl(area);
    setupBottomArea(area, currentWidth);
    volumeFader.setBounds(area.reduced(2));
}

void PerformanceSlotItem::setupTopArea(juce::Rectangle<int>& area, int currentWidth)
{
    juce::Font maxFont(16.0f);
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
    bool hasCustomName = nameLabel.getText().isNotEmpty();
    int textWidth = sharedFont.getStringWidth(nameLabel.getText());
    bool fits = currentWidth >= textWidth;

    nameLabel.setVisible(hasCustomName && fits);
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
    groupLabel.setFont(sharedFont);
    groupLabel.setBounds(topArea.removeFromTop(labelHeight));
}

void PerformanceSlotItem::injectPanControl(juce::Rectangle<int>& area)
{
    if (isStereoMain) {
        int panHeight = juce::jmax(35, (int)(getLocalBounds().getHeight() * 0.09f));
        auto panArea = area.removeFromTop(panHeight);

        panSlider.setBounds(panArea.reduced(5));
        panSlider.setVisible(true);
    }
    else {
        panSlider.setVisible(false);
    }
}

void PerformanceSlotItem::setupBottomArea(juce::Rectangle<int>& area, int currentWidth)
{
    auto bottomArea = area.removeFromBottom(25);
    valueLabel.setFont(sharedFont);
    int requiredWidth = sharedFont.getStringWidth("-88.8 dB");
    valueLabel.setVisible(currentWidth >= requiredWidth);
    valueLabel.setBounds(bottomArea.reduced(2, 0));
}