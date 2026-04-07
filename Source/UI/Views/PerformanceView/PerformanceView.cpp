#include "PerformanceView.h"

PerformanceView::PerformanceView(KaiCBFaderControlAudioProcessor& p)
	:processor(p)
{
	setLookAndFeel(&performanceLF);
	createFaderSlots();

}

void PerformanceView::createFaderSlots()
{
	for (int i = 0; i < 32; ++i)
	{
		auto* slot = slots.add(new PerformanceSlotItem(processor, i + 1));
		addAndMakeVisible(slot);
	}
}

PerformanceView::~PerformanceView()
{
	setLookAndFeel(nullptr);
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

	if (slots.size() > 0)
	{
		layoutSlots(area);
	}
}

void PerformanceView::layoutSlots(juce::Rectangle<int>& area)
{
	int slotWidth = area.getWidth() / slots.size();

	for (auto* slot : slots)
	{
		slot->setBounds(area.removeFromLeft(slotWidth));
	}
}

