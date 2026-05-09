#include "SlotConfigItem.h"
#include "../../../Main/SlotIDs.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../../Utils/StateUtils/SlotStateHelpers.h"

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
	addAndMakeVisible(slotLabel);
	slotLabel.setText("Slot " + juce::String(slotNumber), juce::dontSendNotification);
	slotLabel.setJustificationType(juce::Justification::centred);
	slotLabel.setInterceptsMouseClicks(false, false);
}

void SlotConfigItem::configNameEditor(int slotNumber)
{
	addAndMakeVisible(customNameEditor);
	customNameEditor.setColour(juce::TextEditor::textColourId, MyColours::black);
	customNameEditor.setTextToShowWhenEmpty("Text here...", juce::Colour::greyLevel(0.75f));
	customNameEditor.setJustification(juce::Justification::left);

	auto currentName = SlotStateHelpers::getSlotCustomName(processor.apvts.state, slotNumber);
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
				SlotStateHelpers::setSlotCustomName(processor.apvts.state, slotNumber, newText);
			}
			else
			{
				SlotStateHelpers::setSlotCustomName(processor.apvts.state, slotNumber, "");
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
				processor.globalSlotRegistry->claimSlot(slotNumber, processor.getInstanceId());
			else
				processor.globalSlotRegistry->releaseSlot(slotNumber, processor.getInstanceId());

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
		processor.globalSlotRegistry->claimSlot(slotNumber, processor.getInstanceId());
	else
		processor.globalSlotRegistry->releaseSlot(slotNumber, processor.getInstanceId());

	auto notification = shouldNotify ? juce::sendNotificationSync : juce::dontSendNotification;
	activeToggle.setToggleState(shouldBeActive, notification);
}

bool SlotConfigItem::isActive() const
{
	return activeToggle.getToggleState();
}

void SlotConfigItem::setSelected(bool shouldBeSelected)
{
	if (isLassoSelected != shouldBeSelected)
	{
		isLassoSelected = shouldBeSelected;
		repaint();
	}
}

void SlotConfigItem::paint(juce::Graphics& g)
{
	g.setColour(juce::Colours::darkgrey);
	g.drawRect(getLocalBounds(), 1);

	if (isLassoSelected)
	{
		auto bounds = getLocalBounds().toFloat();
		float cornerSize = 4.0f;

		g.setColour(juce::Colours::white.withAlpha(0.08f));
		g.fillRoundedRectangle(bounds, cornerSize);

		g.setColour(juce::Colours::white.withAlpha(0.3f));
		g.drawRoundedRectangle(bounds, cornerSize, 1.5f);
	}
}

void SlotConfigItem::resized()
{
	juce::Rectangle<int> area = getLocalBounds().reduced(5);

	activeToggle.setBounds(area.removeFromLeft(25));
	slotLabel.setBounds(area.removeFromLeft(40));
	customNameEditor.setBounds(area);
}

void SlotConfigItem::mouseDown(const juce::MouseEvent& e)
{
	if (onBackgroundMouseDown)
		onBackgroundMouseDown(e);
}

void SlotConfigItem::mouseDrag(const juce::MouseEvent& e)
{
	if (onBackgroundMouseDrag)
		onBackgroundMouseDrag(e);
}

void SlotConfigItem::mouseUp(const juce::MouseEvent& e)
{
	if (onBackgroundMouseUp)
		onBackgroundMouseUp(e);
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
