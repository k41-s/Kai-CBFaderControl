#include "SlotConfigItem.h"

void SlotConfigItem::configSlotLabel(int slotNumber)
{
	slotLabel.setText("Slot " + juce::String(slotNumber), juce::dontSendNotification);
	slotLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(slotLabel);
}

void SlotConfigItem::configNameEditor()
{
	customNameEditor.setTextToShowWhenEmpty("Text here...", juce::Colour::greyLevel(0.4f));
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
	auto notification = shouldNotify ? juce::sendNotification : juce::dontSendNotification;
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
