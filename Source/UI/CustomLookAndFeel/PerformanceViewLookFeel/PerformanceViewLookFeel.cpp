#include "PerformanceViewLookFeel.h"
#include "../MyColours.h"
#include "../../Components/UIConstants.h"

PerformanceViewLookFeel::PerformanceViewLookFeel()
{
	setColour(juce::Slider::trackColourId, MyColours::background);
	setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
}

void PerformanceViewLookFeel::drawLinearSlider(juce::Graphics& g, int x, int y, 
	int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, 
	const juce::Slider::SliderStyle style, juce::Slider& slider
) {
	drawFader(x, y, width, height, g, slider, sliderPos);
}

void PerformanceViewLookFeel::drawFader(int x, int y, int width, int height, juce::Graphics& g, juce::Slider& slider, float sliderPos)
{
	auto area = juce::Rectangle<float>(x, y, width, height);

	drawFaderTrack(g, slider, area);

	float capWidth = juce::jmin(40.0f, area.getWidth() * 0.8f);
	float capHeight = 20.0f;
	auto capBounds = juce::Rectangle<float>(0, 0, capWidth, capHeight).withCentre({ area.getCentreX(), sliderPos });

	bool isHighResMode = slider.getProperties().getWithDefault(UIProperties::isHighRes, false);

	juce::Colour capColour = slider.findColour(juce::Slider::thumbColourId);

	setCapColour(slider, capColour, isHighResMode);
	drawFaderCap(g, capColour, capBounds);
	drawIndicatorLine(g, capBounds);
}

void PerformanceViewLookFeel::drawFaderTrack(juce::Graphics& g, juce::Slider& slider, juce::Rectangle<float>& area)
{
	auto trackWidth = 4.0f;
	g.setColour(slider.findColour(juce::Slider::trackColourId));
	g.fillRoundedRectangle(area.withSizeKeepingCentre(trackWidth, area.getHeight()), 2.0f);
}

void PerformanceViewLookFeel::setCapColour(juce::Slider& slider, juce::Colour& capColour, bool isHighResMode)
{
	if (slider.getProperties().contains(UIProperties::customColour))
		capColour = juce::Colour::fromString(slider.getProperties()[UIProperties::customColour].toString());

	if (isHighResMode)
		capColour = capColour.darker(0.4f);
}

void PerformanceViewLookFeel::drawFaderCap(juce::Graphics& g, const juce::Colour& capColour, const juce::Rectangle<float>& capBounds)
{
	g.setColour(capColour);
	g.fillRoundedRectangle(capBounds, 3.0f);
}

void PerformanceViewLookFeel::drawIndicatorLine(juce::Graphics& g, juce::Rectangle<float>& capBounds)
{
	g.setColour(juce::Colours::black);
	g.drawHorizontalLine(static_cast<int>(capBounds.getCentreY()), capBounds.getX() + 2, capBounds.getRight() - 2);
}

void PerformanceViewLookFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
	float sliderPos, const float startAngle, const float endAngle, juce::Slider& slider
) {
	auto outline = juce::Rectangle<float>(x, y, width, height).reduced(2.0f);
	auto radius = juce::jmin(outline.getWidth(), outline.getHeight()) / 2.0f;
	auto centreX = x + width * 0.5f;
	auto centreY = y + height * 0.5f;
	auto angle = startAngle + sliderPos * (endAngle - startAngle);

	drawKnobBackground(g, centreX, radius, centreY);
	configAndDrawIndicatorPointer(radius, angle, centreX, centreY, g);
	drawOuterRing(g, centreX, radius, centreY);
}

void PerformanceViewLookFeel::drawKnobBackground(juce::Graphics& g, float centreX, float radius, float centreY)
{
	g.setColour(MyColours::unpressedBtn);
	g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
}

void PerformanceViewLookFeel::configAndDrawIndicatorPointer(float radius, float angle, float centreX, float centreY, juce::Graphics& g)
{
	juce::Path p;
	configIndicatorPointer(radius, p, angle, centreX, centreY);
	drawIndicatorPointer(g, p);
}

void PerformanceViewLookFeel::configIndicatorPointer(float radius, juce::Path& p, float angle, float centreX, float centreY)
{
	auto pointerLength = radius * 0.8f;
	auto pointerThickness = 2.0f;
	p.addRoundedRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength, 1.0f);
	p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
}

void PerformanceViewLookFeel::drawIndicatorPointer(juce::Graphics& g, juce::Path& p)
{
	g.setColour(juce::Colours::white);
	g.fillPath(p);
}

void PerformanceViewLookFeel::drawOuterRing(juce::Graphics& g, float centreX, float radius, float centreY)
{
	g.setColour(juce::Colours::black.withAlpha(0.5f));
	g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.0f);
}

void PerformanceViewLookFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, 
	const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, 
	bool shouldDrawButtonAsDown
) {
	auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);

	juce::Colour bgColour = MyColours::unpressedBtn;

	getColourFromToggleState(button, bgColour);
	drawButton(g, bgColour, bounds);
}

void PerformanceViewLookFeel::getColourFromToggleState(juce::Button& button, juce::Colour& bgColour)
{
	if (button.getToggleState())
	{
		if (button.getName() == UIComponentNames::muteButton)
			bgColour = juce::Colours::red.withAlpha(0.8f);
		else if (button.getName() == UIComponentNames::soloButton)
			bgColour = juce::Colours::gold.withAlpha(0.8f);
		else bgColour = juce::Colours::orange;
	}
}

void PerformanceViewLookFeel::drawButton(juce::Graphics& g, const juce::Colour& bgColour, const juce::Rectangle<float>& bounds)
{
	g.setColour(bgColour);
	g.fillRoundedRectangle(bounds, 3.0f);

	g.setColour(juce::Colours::black);
	g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
}

void PerformanceViewLookFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, 
	bool isMouseOverButton, bool isButtonDown
) {
	g.setColour(button.getToggleState() ? juce::Colours::black : juce::Colours::white);

	float fontSize = juce::jmin(14.0f, button.getHeight() * 0.6f);
	g.setFont(juce::Font(fontSize, juce::Font::bold));

	g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, false);
}
