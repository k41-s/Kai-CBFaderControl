#include "SlotConfigItem.h"
#include "../../../Main/SlotIDs.h"
#include "../../CustomLookAndFeel/MyColours.h"

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
	customNameEditor.setColour(juce::TextEditor::textColourId, MyColours::black);
	customNameEditor.setTextToShowWhenEmpty("Text here...", juce::Colour::greyLevel(0.75f));
	customNameEditor.setJustification(juce::Justification::left);

	auto currentName = processor.apvts.state.getProperty(SlotIDs::slotName(slotNumber), "");
	customNameEditor.setText(currentName, juce::dontSendNotification);

	configSaveCustomName(slotNumber);
}

void SlotConfigItem::configSaveCustomName(int slotNumber)
{
	auto saveText = [this, slotNumber]()
		{
			auto newText = customNameEditor.getText().trim();
			auto defaultName = "Slot " + juce::String(slotNumber);

			if (newText.isNotEmpty() && newText != defaultName)
			{
				processor.apvts.state.setProperty(SlotIDs::slotName(slotNumber), newText, nullptr);
			}
			else
			{
				processor.apvts.state.removeProperty(SlotIDs::slotName(slotNumber), nullptr);

				customNameEditor.setText("", juce::dontSendNotification);
			}
		};

	customNameEditor.onReturnKey = saveText;
	customNameEditor.onFocusLost = saveText;
}

void SlotConfigItem::configActiveToggle()
{
	addAndMakeVisible(activeToggle);
	activeToggle.onClick = [this]
		{
			if (activeToggle.getToggleState())
			{
				// 1. User is trying to manually turn the slot ON
				if (!processor.globalSlotRegistry->claimSlot(slotNumber, &processor))
				{
					// Failed! Another instance owns it. Show a warning.
					juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
						"Slot Unavailable",
						"Slot " + juce::String(slotNumber) + " is already active in another plugin instance.");

					// Force the toggle back OFF and tell the APVTS to update
					activeToggle.setToggleState(false, juce::sendNotificationSync);
					return; // Abort here so we don't trigger the onToggleChanged callback!
				}
			}
			else
			{
				// 2. User is manually turning it OFF, so release it back to the global pool
				processor.globalSlotRegistry->releaseSlot(slotNumber, &processor);
			}

			// If we got this far, the claim was successful (or it was an OFF command)
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
	if (shouldBeActive)
	{
		// 1. "Toggle All" is trying to turn this ON
		if (!processor.globalSlotRegistry->claimSlot(slotNumber, &processor))
		{
			// Slot is taken by another instance! 
			// We just silently return and ignore the command so we don't steal it.
			return;
		}
	}
	else
		processor.globalSlotRegistry->releaseSlot(slotNumber, &processor);

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
		auto newName = tree.getProperty(property, "").toString();
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
