#include "PerformanceViewLookFeel.h"
#include "../MyColours.h"
#include "../../Components/UIConstants.h"

PerformanceViewLookFeel::PerformanceViewLookFeel()
{
	setColour(juce::Slider::trackColourId, MyColours::faderTrack);
	setColour(juce::Slider::thumbColourId, MyColours::faderCap);
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

	float tickAreaWidth = juce::jlimit(20.0f, 30.0f, area.getWidth() * 0.30f);
	auto tickArea = area.removeFromLeft(tickAreaWidth);

	area.removeFromLeft(2.0f);

	drawFaderScale(g, slider, tickArea);
	drawFaderTrack(g, slider, area);

	auto capBounds = getFaderCapBounds(area, sliderPos);
	bool isHighResMode = slider.getProperties().getWithDefault(UIProperties::isHighRes, UIProperties::defaultHighRes);
	juce::Colour capColour = slider.findColour(juce::Slider::thumbColourId);

	setCapColour(slider, capColour, isHighResMode);
	drawFaderCap(g, capColour, capBounds);
	drawIndicatorLine(g, capBounds);
}

void PerformanceViewLookFeel::drawFaderScale(juce::Graphics& g, juce::Slider& slider, const juce::Rectangle<float>& tickArea)
{
	if (slider.getHeight() <= 0) return;

	std::vector<double> tickValues = { 20.0, 10.0, 5.0, 0.0, -5.0, -10.0, -20.0, -30.0, -40.0, -50.0, -75.0, inf };

	float fontSize = juce::jlimit(10.0f, 13.0f, tickArea.getWidth() * 0.5f);
	g.setFont(juce::Font(fontSize));

	drawTickValues(tickValues, slider, tickArea, g);
}

void PerformanceViewLookFeel::drawTickValues(std::vector<double>& tickValues, juce::Slider& slider, const juce::Rectangle<float>& tickArea, juce::Graphics& g)
{
	for (double val : tickValues)
	{
		float y = (float)slider.getPositionOfValue(val);

		bool isZero = (val == 0.0);
		bool isInf = (val == inf);

		float lineLength = isZero ? 4.0f : 2.0f;
		drawTickLine(tickArea, lineLength, g, isZero, y);
		drawTickText(isZero, isInf, val, tickArea, y, lineLength, g);
	}
}

void PerformanceViewLookFeel::drawTickLine(const juce::Rectangle<float>& tickArea, float lineLength, juce::Graphics& g, bool isZero, float y)
{
	float lineX = tickArea.getRight() - lineLength;
	g.setColour(isZero ? juce::Colours::white : juce::Colours::grey);
	g.drawHorizontalLine(static_cast<int>(y), lineX, tickArea.getRight());
}

void PerformanceViewLookFeel::drawTickText(bool isZero, bool isInf, double val, const juce::Rectangle<float>& tickArea, float y, float lineLength, juce::Graphics& g)
{
	juce::String text;
	if (isZero) text = "0";
	else if (isInf) text = juce::String::charToString(0x221E); // inf symbol hex
	else text = juce::String((int)val);

	auto textBounds = juce::Rectangle<float>(tickArea.getX(), y - 5.0f, tickArea.getWidth() - lineLength - 2.0f, 10.0f);

	g.setColour(isZero ? juce::Colours::white : juce::Colours::lightgrey.withAlpha(0.8f));
	g.drawText(text, textBounds, juce::Justification::centredRight, false);
}

void PerformanceViewLookFeel::drawFaderTrack(juce::Graphics& g, juce::Slider& slider, juce::Rectangle<float>& area)
{
	auto trackWidth = 5.0f;
	g.setColour(slider.findColour(juce::Slider::trackColourId));
	g.fillRoundedRectangle(area.withSizeKeepingCentre(trackWidth, area.getHeight()), 2.0f);
}

juce::Rectangle<float> PerformanceViewLookFeel::getFaderCapBounds(juce::Rectangle<float>& area, float sliderPos)
{
	float capWidth = juce::jlimit(12.0f, 24.0f, area.getWidth() * 0.7f);
	float targetCapHeight = capWidth * 2.0f;
	float maxAllowedHeight = area.getHeight() * 0.15f;
	float capHeight = juce::jmax(16.0f, juce::jmin(targetCapHeight, maxAllowedHeight));

	return juce::Rectangle<float>(0, 0, capWidth, capHeight).withCentre({ area.getCentreX(), sliderPos });
}

void PerformanceViewLookFeel::setCapColour(juce::Slider& slider, juce::Colour& capColour, bool isHighResMode)
{
	if (slider.getProperties().contains(UIProperties::customColour))
		capColour = juce::Colour::fromString(slider.getProperties()[UIProperties::customColour].toString());

	if (isHighResMode)
		capColour = capColour.darker(0.7f);
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
	int numRidgesTotal = 10;
	float ridgeSpacing = capBounds.getHeight() / numRidgesTotal;

	for (int i = 1; i < numRidgesTotal; ++i)
	{
		if (i == numRidgesTotal / 2) continue;

		float ridgeY = capBounds.getY() + (i * ridgeSpacing);

		g.setColour(juce::Colours::black.withAlpha(0.3f));
		g.drawHorizontalLine(static_cast<int>(ridgeY), capBounds.getX() + 1.0f, capBounds.getRight() - 1.0f);

		g.setColour(juce::Colours::white.withAlpha(0.2f));
		g.drawHorizontalLine(static_cast<int>(ridgeY + 1.0f), capBounds.getX() + 1.0f, capBounds.getRight() - 1.0f);
	}
}

void PerformanceViewLookFeel::drawFaderCapSideShadow(const juce::Rectangle<float>& capBounds, juce::Graphics& g)
{
	juce::ColourGradient sideShadow;
	configSideShadow(sideShadow, capBounds);
	g.setGradientFill(sideShadow);
	g.fillRect(capBounds);
}

void PerformanceViewLookFeel::configSideShadow(juce::ColourGradient& sideShadow, const juce::Rectangle<float>& capBounds)
{
	sideShadow.point1 = { capBounds.getX(), 0.0f };
	sideShadow.point2 = { capBounds.getRight(), 0.0f };
	sideShadow.addColour(0.0f, juce::Colours::black.withAlpha(0.3f));
	sideShadow.addColour(0.15f, juce::Colours::transparentBlack);
	sideShadow.addColour(0.85f, juce::Colours::transparentBlack);
	sideShadow.addColour(1.0f, juce::Colours::black.withAlpha(0.3f));
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