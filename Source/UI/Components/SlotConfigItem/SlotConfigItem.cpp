#include "SlotConfigItem.h"
#include "../../../Main/SlotIDs.h"

void SlotConfigItem::configSlotLabel(int slotNumber)
{
	slotLabel.setText("Slot " + juce::String(slotNumber), juce::dontSendNotification);
	slotLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(slotLabel);
}

void SlotConfigItem::configNameEditor()
{
	customNameEditor.setTextToShowWhenEmpty("Text here...", juce::Colour::greyLevel(0.6f));
	customNameEditor.setJustification(juce::Justification::left);
	addAndMakeVisible(customNameEditor);
}

SlotConfigItem::SlotConfigItem(int slotNumber)
{
	configSlotLabel(slotNumber);

	configNameEditor();

	addAndMakeVisible(activeToggle);

	activeToggle.onClick = [this]
		{
			if (onToggleChanged != nullptr)
				onToggleChanged();
		};
}

SlotConfigItem::~SlotConfigItem()
{
}

void SlotConfigItem::setToggleState(bool shouldBeActive, bool shouldNotify)
{
	auto notification = shouldNotify ? juce::sendNotificationSync : juce::dontSendNotification;
	activeToggle.setToggleState(shouldBeActive, notification);
}

bool SlotConfigItem::isActive() const
{
	return activeToggle.getToggleState();
}

void SlotConfigItem::paint(juce::Graphics& g)
{
	g.setColour(juce::Colours::darkgrey);
	g.drawRect(getLocalBounds(), 1);
}

void SlotConfigItem::resized()
{
	juce::Rectangle<int> area = getLocalBounds().reduced(5);

	activeToggle.setBounds(area.removeFromLeft(25));
	slotLabel.setBounds(area.removeFromLeft(40));
	customNameEditor.setBounds(area);
}

void SlotConfigItem::setupAttachment(juce::AudioProcessorValueTreeState& state, int slotNum)
{
	activeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		state,
		SlotIDs::isActive(slotNum),
		activeToggle
	);
}
