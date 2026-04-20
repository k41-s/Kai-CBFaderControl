#include "PerformanceViewLookFeel.h"
#include "../MyColours.h"
#include "../../Components/UIConstants.h"

PerformanceViewLookFeel::PerformanceViewLookFeel()
{
	setColour(juce::Slider::trackColourId, MyColours::faderTrack);
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

	float capWidth = juce::jlimit(12.0f, 23.0f, area.getWidth() * 0.6f);

	float targetCapHeight = capWidth * 2.0f;
	float maxAllowedHeight = area.getHeight() * 0.15f;
	float capHeight = juce::jmax(16.0f, juce::jmin(targetCapHeight, maxAllowedHeight));

	auto capBounds = juce::Rectangle<float>(0, 0, capWidth, capHeight).withCentre({ area.getCentreX(), sliderPos });

	bool isHighResMode = slider.getProperties().getWithDefault(UIProperties::isHighRes, UIProperties::defaultHighRes);

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
		capColour = capColour.darker(0.6f);
}

void PerformanceViewLookFeel::drawFaderCap(juce::Graphics& g, const juce::Colour& capColour, const juce::Rectangle<float>& capBounds)
{
	drawFaderCapMainGradient(capBounds, capColour, g);
	drawFaderCapRidges(capBounds, g);
	drawFaderCapSideShadow(capBounds, g);
	drawFaderCapOuterBorder(g, capBounds);
}

void PerformanceViewLookFeel::drawFaderCapMainGradient(const juce::Rectangle<float>& capBounds, const juce::Colour& capColour, juce::Graphics& g)
{
	juce::ColourGradient mainGradient = getFaderCapMainGradient(capBounds, capColour);

	g.setGradientFill(mainGradient);
	g.fillRect(capBounds);
}

juce::ColourGradient PerformanceViewLookFeel::getFaderCapMainGradient(const juce::Rectangle<float>& capBounds, const juce::Colour& capColour)
{
	juce::ColourGradient gradient;
	gradient.point1 = { capBounds.getX(), capBounds.getY() };
	gradient.point2 = { capBounds.getX(), capBounds.getBottom() };
	gradient.isRadial = false;

	gradient.addColour(0.0f, capColour.brighter(0.8f));
	gradient.addColour(0.48f, capColour.darker(0.3f));
	gradient.addColour(0.52f, capColour.brighter(0.2f));
	gradient.addColour(1.0f, capColour.darker(0.9f));

	return gradient;
}

void PerformanceViewLookFeel::drawFaderCapRidges(const juce::Rectangle<float>& capBounds, juce::Graphics& g)
{
	// 2. Draw subtle etched ridges instead of separate stacked blocks
	int numRidgesTotal = 10;
	float ridgeSpacing = capBounds.getHeight() / numRidgesTotal;

	for (int i = 1; i < numRidgesTotal; ++i)
	{
		// Skip the exact center where the indicator groove goes
		if (i == numRidgesTotal / 2) continue;

		float ridgeY = capBounds.getY() + (i * ridgeSpacing);

		// Subtle dark indent
		g.setColour(juce::Colours::black.withAlpha(0.3f));
		g.drawHorizontalLine(static_cast<int>(ridgeY), capBounds.getX() + 1.0f, capBounds.getRight() - 1.0f);

		// Subtle light highlight below indent to give depth (simulates a physical scratch/etch)
		g.setColour(juce::Colours::white.withAlpha(0.2f));
		g.drawHorizontalLine(static_cast<int>(ridgeY + 1.0f), capBounds.getX() + 1.0f, capBounds.getRight() - 1.0f);
	}
}

void PerformanceViewLookFeel::drawFaderCapSideShadow(const juce::Rectangle<float>& capBounds, juce::Graphics& g)
{
	// 3. Optional: Inner side shadows to give it a slightly rounded 3D cylinder feel on the edges
	juce::ColourGradient sideShadow;
	sideShadow.point1 = { capBounds.getX(), 0.0f };
	sideShadow.point2 = { capBounds.getRight(), 0.0f };
	sideShadow.addColour(0.0f, juce::Colours::black.withAlpha(0.3f));
	sideShadow.addColour(0.15f, juce::Colours::transparentBlack);
	sideShadow.addColour(0.85f, juce::Colours::transparentBlack);
	sideShadow.addColour(1.0f, juce::Colours::black.withAlpha(0.3f));
	g.setGradientFill(sideShadow);
	g.fillRect(capBounds);
}

void PerformanceViewLookFeel::drawFaderCapOuterBorder(juce::Graphics& g, const juce::Rectangle<float>& capBounds)
{
	g.setColour(juce::Colours::black.withAlpha(0.8f));
	g.drawRect(capBounds, 1.0f);
}

void PerformanceViewLookFeel::drawIndicatorLine(juce::Graphics& g, juce::Rectangle<float>& capBounds)
{
	float cy = capBounds.getCentreY();
	float left = capBounds.getX();
	float right = capBounds.getRight();

	float gapHeight = 2.0f;
	auto gapBounds = juce::Rectangle<float>(left, cy - gapHeight * 0.5f, capBounds.getWidth(), gapHeight);

	g.setColour(juce::Colours::black.withAlpha(0.6f));
	g.fillRect(gapBounds);

	g.setColour(juce::Colours::black);
	g.drawHorizontalLine(static_cast<int>(gapBounds.getY()), left, right);

	g.setColour(juce::Colours::white.withAlpha(0.3f));
	g.drawHorizontalLine(static_cast<int>(gapBounds.getBottom()), left + 1.0f, right - 1.0f);
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
	const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown
) {
	auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
	float cornerSize = 4.0f;

	juce::Colour bgColour = MyColours::unpressedBtn;

	getColourFromToggleState(button, bgColour);
	drawButton(g, bgColour, bounds, cornerSize);
	handleMouseOverButton(isMouseOverButton, isButtonDown, g, bounds, cornerSize);
}

void PerformanceViewLookFeel::getColourFromToggleState(juce::Button& button, juce::Colour& colourToChange)
{
	if (button.getToggleState())
	{
		if (button.getName() == UIComponentNames::muteButton)
			colourToChange = juce::Colours::red.withAlpha(0.8f);
		else if (button.getName() == UIComponentNames::soloButton)
			colourToChange = juce::Colours::gold.withAlpha(0.8f);
		else colourToChange = juce::Colours::orange;
	}
}

void PerformanceViewLookFeel::drawButton(juce::Graphics& g, const juce::Colour& bgColour, const juce::Rectangle<float>& bounds, float cornerSize)
{
	g.setColour(bgColour);
	g.fillRoundedRectangle(bounds, cornerSize);

	g.setColour(juce::Colours::black);
	g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
}

void PerformanceViewLookFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, 
	bool isMouseOverButton, bool isButtonDown
) {
	g.setColour(button.getToggleState() ? juce::Colours::black : juce::Colours::white);

	float fontSize = juce::jmin(14.0f, button.getWidth() * 0.6f);
	g.setFont(juce::Font(fontSize, juce::Font::bold));

	g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, false);
}

void PerformanceViewLookFeel::handleMouseOverButton(bool isMouseOverButton, bool isButtonDown, juce::Graphics& g, 
	juce::Rectangle<float>& area, float cornerSize
) {
	if (isMouseOverButton && !isButtonDown)
	{
		g.setColour(MyColours::mouseOverButton);
		g.fillRoundedRectangle(area.reduced(1.0f), cornerSize);
	}
}