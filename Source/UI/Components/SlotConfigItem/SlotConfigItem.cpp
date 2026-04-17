#include "SlotConfigItem.h"
#include "../../../Main/SlotIDs.h"


SlotConfigItem::SlotConfigItem(KaiCBFaderControlAudioProcessor& p, int slotNum)
	: processor(p), slotNumber(slotNum)
{
	processor.apvts.state.addListener(this);

	configSlotLabel(slotNumber);
	configNameEditor(slotNumber);
	configActiveToggle();
}


void SlotConfigItem::configSlotLabel(int slotNumber)
{
	slotLabel.setText("Slot " + juce::String(slotNumber), juce::dontSendNotification);
	slotLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(slotLabel);
}

void SlotConfigItem::configNameEditor(int slotNumber)
{
	addAndMakeVisible(customNameEditor);
	customNameEditor.setTextToShowWhenEmpty("Text here...", juce::Colour::greyLevel(0.6f));
	customNameEditor.setJustification(juce::Justification::left);

	auto currentName = processor.apvts.state.getProperty(SlotIDs::slotName(slotNumber), "");
	customNameEditor.setText(currentName, juce::dontSendNotification);

	auto saveText = [this, slotNumber]()
		{
			processor.apvts.state.setProperty(SlotIDs::slotName(slotNumber),
				customNameEditor.getText(),
				nullptr);
		};

	customNameEditor.onReturnKey = saveText;
	customNameEditor.onFocusLost = saveText;
}

void SlotConfigItem::configActiveToggle()
{
	addAndMakeVisible(activeToggle);
	activeToggle.onClick = [this]
		{
			if (onToggleChanged != nullptr)
				onToggleChanged();
		};
}

SlotConfigItem::~SlotConfigItem()
{
	processor.apvts.state.removeListener(this);
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

void SlotConfigItem::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
	if (property == juce::Identifier(SlotIDs::slotName(slotNumber)))
	{
		auto newName = tree.getProperty(property).toString();
		setNewNameIfDifferent(newName);
	}
}

void SlotConfigItem::setNewNameIfDifferent(juce::String& newName)
{
	if (customNameEditor.getText() != newName)
	{
		customNameEditor.setText(newName, juce::dontSendNotification);
	}
}

void SlotConfigItem::setupAttachment(juce::AudioProcessorValueTreeState& state, int slotNum)
{
	activeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		state,
		SlotIDs::isActive(slotNum),
		activeToggle
	);
}
