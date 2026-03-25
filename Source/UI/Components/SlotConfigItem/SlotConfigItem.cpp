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
}

SlotConfigItem::~SlotConfigItem()
{
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
