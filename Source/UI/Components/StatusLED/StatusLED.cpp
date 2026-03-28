#include "StatusLED.h"

StatusLED::StatusLED()
{
}

StatusLED::~StatusLED()
{
}

void StatusLED::setConnected(bool connected)
{
	if (isConnected != connected) {
		isConnected = connected;
		repaint();
	}
}

void StatusLED::paint(juce::Graphics& g)
{
	auto area = getLocalBounds().toFloat().reduced(2.0f);

	auto colour = isConnected ? juce::Colours::limegreen : juce::Colours::red;

	g.setColour(colour.withAlpha(0.3f));
	g.fillEllipse(area);

	g.setColour(colour);
	g.fillEllipse(area.reduced(4.0f));
}
