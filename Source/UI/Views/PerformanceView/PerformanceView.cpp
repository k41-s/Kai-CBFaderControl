#include "PerformanceView.h"
#include "../../../Main/SlotIDs.h"

PerformanceView::PerformanceView(KaiCBFaderControlAudioProcessor& p)
	:processor(p)
{
	setLookAndFeel(&performanceLF);
	createFaderSlots();

	registerIsActiveListener();
	triggerAsyncUpdate();
}

void PerformanceView::createFaderSlots()
{
	for (int i = 0; i < 32; ++i)
	{
		auto* slot = slots.add(new PerformanceSlotItem(processor, i + 1));
		addAndMakeVisible(slot);
	}
}

void PerformanceView::registerIsActiveListener()
{
	for (int i = 1; i <= 32; ++i)
	{
		processor.apvts.addParameterListener(SlotIDs::isActive(i), this);
	}
}

PerformanceView::~PerformanceView()
{
	deregisterIsActiveListener();
	setLookAndFeel(nullptr);
}

void PerformanceView::deregisterIsActiveListener()
{
	for (int i = 1; i <= 32; ++i)
	{
		processor.apvts.removeParameterListener(SlotIDs::isActive(i), this);
	}
}

void PerformanceView::parameterChanged(const juce::String& parameterID, float newValue)
{
	triggerAsyncUpdate();
}

void PerformanceView::handleAsyncUpdate()
{
	resized();
}

void PerformanceView::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colour(0xFF121212));
}

void PerformanceView::resized()
{
	setupAndFillArea();
}

void PerformanceView::setupAndFillArea()
{
	auto area = getLocalBounds();
	juce::FlexBox flexBox = configFlexBox();
	checkAndAddActiveSlots(flexBox);
	flexBox.performLayout(area);
}

juce::FlexBox PerformanceView::configFlexBox()
{
	juce::FlexBox flexBox;
	flexBox.flexDirection = juce::FlexBox::Direction::row;
	flexBox.flexWrap = juce::FlexBox::Wrap::noWrap;
	flexBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
	return flexBox;
}

void PerformanceView::checkAndAddActiveSlots(juce::FlexBox& flexBox)
{
	for (int i = 0; i < slots.size(); ++i)
	{
		auto* slot = slots[i];

		bool isActive = *processor.isActiveParams[i] > 0.5f;
		slot->setVisible(isActive);

		addSlotIfActive(isActive, flexBox, slot);
	}
}

void PerformanceView::addSlotIfActive(bool isActive, juce::FlexBox& flexBox, PerformanceSlotItem* slot)
{
	if (isActive)
		flexBox.items.add(juce::FlexItem(*slot)
			.withMinWidth(60.0f)
			.withMaxWidth(120.0f)
			.withFlex(1.0f));
}
