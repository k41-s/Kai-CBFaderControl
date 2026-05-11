#include "PerformanceViewLookFeel.h"
#include "../MyColours.h"
#include "../../Components/UIConstants.h"

PerformanceViewLookFeel::PerformanceViewLookFeel()
{
	setColour(juce::Slider::trackColourId, MyColours::faderTrack);
	setColour(juce::Slider::thumbColourId, MyColours::faderCap);

	setColour(juce::LassoComponent<int>::lassoFillColourId, MyColours::cbBlue.withAlpha(0.1f));
	setColour(juce::LassoComponent<int>::lassoOutlineColourId, MyColours::cbBlue.withAlpha(0.8f));
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

	float tickAreaWidth = juce::jlimit(
		UISizeConstants::tickAreaMinWidth, 
		UISizeConstants::tickAreaMaxWidth, 
		area.getWidth() * UILayoutPercentages::tickAreaWidthPct);
	auto tickArea = area.removeFromLeft(tickAreaWidth);

	area.removeFromLeft(2.0f);

	drawFaderScale(g, slider, tickArea);
	drawFaderTrack(g, slider, area);

	auto capBounds = getFaderCapBounds(area, sliderPos);
	bool isHighResMode = slider.getProperties().getWithDefault(UIProperties::isHighRes, UIProperties::defaultHighRes);
	juce::Colour capColour = slider.findColour(juce::Slider::thumbColourId);

	setCapColour(slider, capColour, isHighResMode);
	drawFaderCap(g, capColour, capBounds);
	drawIndicatorLine(g, capBounds, slider);
}

void PerformanceViewLookFeel::drawFaderScale(juce::Graphics& g, juce::Slider& slider, const juce::Rectangle<float>& tickArea)
{
	if (slider.getHeight() <= 0) return;

	std::vector<double> tickValues = { 20.0, 10.0, 5.0, 0.0, -5.0, -10.0, -20.0, -30.0, -40.0, -50.0, -75.0, inf };

	float fontSize = juce::jlimit(
		UISizeConstants::faderScaleMinFont, 
		UISizeConstants::faderScaleMaxFont, 
		tickArea.getWidth() * 0.5f);
	g.setFont(juce::Font(fontSize));

	drawTickValues(tickValues, slider, tickArea, g);
}

void PerformanceViewLookFeel::drawTickValues(std::vector<double>& tickValues, juce::Slider& slider, const juce::Rectangle<float>& tickArea, juce::Graphics& g)
{
	for (double val : tickValues)
	{
		float y = (float)slider.getPositionOfValue(val);

		bool isZero = (val == 0.0);
		bool isInf = (val <= inf);

		float maxLineLength = isZero ? 4.0f : 2.0f;
		float widthPercentage = isZero ? 0.2f : 0.1f;

		float lineLength = juce::jmin(maxLineLength, tickArea.getWidth() * widthPercentage);
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
	else if (isInf) text = UIStringConstants::infSymbol;
	else text = juce::String((int)val);

	auto textBounds = juce::Rectangle<float>(tickArea.getX(), y - 5.0f, tickArea.getWidth() - lineLength - 2.0f, 10.0f);

	g.setColour(isZero ? juce::Colours::white : juce::Colours::lightgrey.withAlpha(0.8f));
	g.drawText(text, textBounds, juce::Justification::centredRight, false);
}

void PerformanceViewLookFeel::drawFaderTrack(juce::Graphics& g, juce::Slider& slider, juce::Rectangle<float>& area)
{
	float trackWidth = juce::jlimit(
		UISizeConstants::faderTrackMinWidth,
		UISizeConstants::faderTrackMaxWidth, 
		area.getWidth() * UILayoutPercentages::faderTrackWidthPct);
	g.setColour(slider.findColour(juce::Slider::trackColourId));
	g.fillRoundedRectangle(area.withSizeKeepingCentre(trackWidth, area.getHeight()), trackWidth * 0.5f);
}

juce::Rectangle<float> PerformanceViewLookFeel::getFaderCapBounds(juce::Rectangle<float>& area, float sliderPos)
{
	float capWidth = juce::jlimit(
		UISizeConstants::faderCapMinWidth, 
		UISizeConstants::faderCapMaxWidth, 
		area.getWidth() * UILayoutPercentages::faderCapWidthPct);
	float targetCapHeight = capWidth * 2.0f;
	float maxAllowedHeight = area.getHeight() * UILayoutPercentages::faderCapMaxHeightPct;
	float capHeight = juce::jmax(UISizeConstants::faderCapMinHeight, juce::jmin(targetCapHeight, maxAllowedHeight));

	return juce::Rectangle<float>(0, 0, capWidth, capHeight).withCentre({ area.getCentreX(), sliderPos });
}

void PerformanceViewLookFeel::setCapColour(juce::Slider& slider, juce::Colour& capColour, bool isHighResMode)
{
	if (slider.getProperties().contains(UIProperties::customColour))
		capColour = juce::Colour::fromString(slider.getProperties()[UIProperties::customColour].toString());
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

void PerformanceViewLookFeel::drawIndicatorLine(juce::Graphics& g, juce::Rectangle<float>& capBounds, juce::Slider& slider)
{
	float cy = capBounds.getCentreY();
	float left = capBounds.getX();
	float right = capBounds.getRight();

	float gapHeight = 2.0f;
	auto gapBounds = juce::Rectangle<float>(left, cy - gapHeight * 0.5f, capBounds.getWidth(), gapHeight);
	if (slider.getProperties().contains(UIProperties::indicatorColour))
	{
		juce::Colour indColour = juce::Colour::fromString(slider.getProperties()[UIProperties::indicatorColour].toString());

		g.setColour(indColour);
		g.fillRect(gapBounds);

		g.setColour(juce::Colours::black.withAlpha(0.4f));
		g.drawHorizontalLine(static_cast<int>(gapBounds.getY()), left, right);
		g.setColour(juce::Colours::white.withAlpha(0.2f));
		g.drawHorizontalLine(static_cast<int>(gapBounds.getBottom()), left, right);
	}
	else 
	{
		g.setColour(juce::Colours::black.withAlpha(0.6f));
		g.fillRect(gapBounds);

		g.setColour(juce::Colours::black);
		g.drawHorizontalLine(static_cast<int>(gapBounds.getY()), left, right);

		g.setColour(juce::Colours::white.withAlpha(0.3f));
		g.drawHorizontalLine(static_cast<int>(gapBounds.getBottom()), left + 1.0f, right - 1.0f);
	}
}

void PerformanceViewLookFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
	float sliderPos, const float startAngle, const float endAngle, juce::Slider& slider
) {
	auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(2.0f);

	float tickAreaWidth = juce::jlimit(
		UISizeConstants::tickAreaMinWidth,
		UISizeConstants::tickAreaMaxWidth, 
		width * UILayoutPercentages::rotaryTickAreaWidthPct);
	auto textArea = bounds.removeFromLeft(tickAreaWidth);
	bounds.removeFromLeft(2.0f);

	drawPanValueText(g, sliderPos, textArea);

	auto outline = bounds.reduced(1.0f);

	float maxDiameter = juce::jlimit(
		UISizeConstants::knobMinDiameter, 
		UISizeConstants::knobMaxDiameter, 
		outline.getWidth() * UILayoutPercentages::knobDiameterPct);
	float diameter = juce::jmin(outline.getWidth(), outline.getHeight() * 0.95f, maxDiameter);

	auto knobArea = outline.withSizeKeepingCentre(diameter, diameter);
	auto radius = knobArea.getWidth() / 2.0f;
	auto centreX = knobArea.getCentreX();
	auto centreY = knobArea.getCentreY();
	auto angle = startAngle + sliderPos * (endAngle - startAngle);

	drawPanScale(g, centreX, centreY, radius, startAngle, endAngle);

	radius -= 2.0f;
	drawKnobBackground(g, centreX, radius, centreY);
	configAndDrawIndicatorPointer(radius, angle, centreX, centreY, g);
	drawOuterRing(g, centreX, radius, centreY);
}

void PerformanceViewLookFeel::drawKnobBackground(juce::Graphics& g, float centreX, float radius, float centreY)
{
	drawKnobDropShadow(g, centreX, centreY, radius);

	juce::ColourGradient baseGradient(juce::Colour(0xFF3A3A3A), centreX - radius * 0.5f, centreY - radius * 0.5f,
		juce::Colour(0xFF151515), centreX + radius * 0.5f, centreY + radius * 0.5f, true);
	g.setGradientFill(baseGradient);
	g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

	drawKnobGrip(g, centreX, centreY, radius);

	float innerRadius = radius * UILayoutPercentages::knobInnerRadiusPct;
	juce::ColourGradient innerGradient(juce::Colour(0xFF282828), centreX, centreY - innerRadius,
		juce::Colour(0xFF1A1A1A), centreX, centreY + innerRadius, false);
	g.setGradientFill(innerGradient);
	g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);
}

void PerformanceViewLookFeel::drawKnobDropShadow(juce::Graphics& g, float centreX, float centreY, float radius)
{
	juce::DropShadow shadow(juce::Colours::black.withAlpha(0.6f), 4, { 0, 3 });
	juce::Path shadowPath;
	shadowPath.addEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
	shadow.drawForPath(g, shadowPath);
}

void PerformanceViewLookFeel::drawKnobGrip(juce::Graphics& g, float centreX, float centreY, float radius)
{
	int numGrips = 36;
	for (int i = 0; i < numGrips; ++i)
	{
		float angle = juce::MathConstants<float>::twoPi * i / numGrips;
		float startX = centreX + (radius * UILayoutPercentages::knobInnerRadiusPct) * std::cos(angle);
		float startY = centreY + (radius * UILayoutPercentages::knobInnerRadiusPct) * std::sin(angle);
		float endX = centreX + radius * std::cos(angle);
		float endY = centreY + radius * std::sin(angle);

		g.setColour(juce::Colours::white.withAlpha(0.12f));
		g.drawLine(startX, startY, endX, endY, 1.5f);

		g.setColour(juce::Colours::black.withAlpha(0.4f));
		g.drawLine(startX + 0.5f, startY + 0.5f, endX + 0.5f, endY + 0.5f, 1.5f);
	}
}

void PerformanceViewLookFeel::configAndDrawIndicatorPointer(float radius, float angle, float centreX, float centreY, juce::Graphics& g)
{
	juce::Path p;
	configIndicatorPointer(radius, p, angle, centreX, centreY);
	drawIndicatorPointer(g, p);
}

void PerformanceViewLookFeel::configIndicatorPointer(float radius, juce::Path& p, float angle, float centreX, float centreY)
{
	auto pointerLength = radius * UILayoutPercentages::knobPointerLengthPct;
	auto pointerThickness = 3.0f;
	p.addRoundedRectangle(-pointerThickness * 0.5f, -radius * 0.78f, pointerThickness, pointerLength, 1.5f);
	p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
}

void PerformanceViewLookFeel::drawIndicatorPointer(juce::Graphics& g, juce::Path& p)
{
	g.setColour(juce::Colours::white.withAlpha(0.9f));
	g.fillPath(p);

	juce::Path highlightPath = p;
	highlightPath.applyTransform(juce::AffineTransform::translation(0.5f, 0.5f));

	g.setColour(juce::Colours::white.withAlpha(0.2f));
	g.strokePath(highlightPath, juce::PathStrokeType(1.0f));
}

void PerformanceViewLookFeel::drawOuterRing(juce::Graphics& g, float centreX, float radius, float centreY)
{
	g.setColour(juce::Colours::white.withAlpha(0.08f));
	g.drawEllipse(centreX - radius - 1.0f, centreY - radius - 1.0f, radius * 2.0f + 2.0f, radius * 2.0f + 2.0f, 1.0f);

	g.setColour(juce::Colours::black.withAlpha(0.5f));
	g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.5f);
}

void PerformanceViewLookFeel::drawPanScale(juce::Graphics& g, float centreX, float centreY, float radius, float startAngle, float endAngle)
{
	g.setColour(juce::Colours::white.withAlpha(0.6f));
	g.setFont(juce::Font(8.5f, juce::Font::bold));

	float scaleRadius = radius + 5.5f;

	drawPanLabels(g, centreX, scaleRadius, centreY, startAngle, endAngle);
	drawPanTicks(g, radius, centreX, centreY, startAngle, endAngle);
}

void PerformanceViewLookFeel::drawPanLabels(juce::Graphics& g, float centreX, float scaleRadius, float centreY, float startAngle, float endAngle)
{
	auto drawLabelAtAngle = [&](float angle, const juce::String& text)
		{
			float tx = centreX + scaleRadius * std::sin(angle);
			float ty = centreY - scaleRadius * std::cos(angle);
			juce::Rectangle<float> r(tx - 10.0f, ty - 6.0f, 20.0f, 12.0f);
			g.drawText(text, r, juce::Justification::centred, false);
		};

	drawLabelAtAngle(startAngle, "L");
	drawLabelAtAngle((startAngle + endAngle) * 0.5f, "C");
	drawLabelAtAngle(endAngle, "R");
}

void PerformanceViewLookFeel::drawPanTicks(juce::Graphics& g, float radius, float centreX, float centreY, float startAngle, float endAngle)
{
	g.setColour(juce::Colours::white.withAlpha(0.2f));

	auto drawTick = [&](float angle)
	{
		float innerR = radius + 1.0f;
		float outerR = radius + 3.5f;
		g.drawLine(centreX + innerR * std::sin(angle), centreY - innerR * std::cos(angle),
			centreX + outerR * std::sin(angle), centreY - outerR * std::cos(angle), 1.0f);
	};

	drawTick(startAngle);
	drawTick((startAngle + endAngle) * 0.5f);
	drawTick(endAngle);
}

void PerformanceViewLookFeel::drawPanValueText(juce::Graphics& g, float sliderPos, juce::Rectangle<float>& textArea)
{
	juce::String panText;

	if (std::abs(sliderPos - 0.5f) < 0.01f)
		panText = "C";
	else
	{
		int pct = juce::roundToInt(std::abs(sliderPos - 0.5f) * 200.0f);
		juce::String side = sliderPos < 0.5f ? "L" : "R";

		if (pct == 100)
			panText = side;
		else
			panText = side + juce::String(pct);
	}

	float fontSize = juce::jlimit(
		UISizeConstants::panTextMinFont, 
		UISizeConstants::panTextMaxFont, 
		textArea.getWidth() * 0.45f);
	g.setFont(juce::Font(fontSize, juce::Font::bold));

	float boxHeight = fontSize + 4.0f;
	auto backgroundArea = textArea.withSizeKeepingCentre(textArea.getWidth(), boxHeight);

	backgroundArea.removeFromRight(1.0f);

	g.setColour(MyColours::valueBackground);
	g.fillRect(backgroundArea);

	g.setColour(MyColours::white);
	g.drawText(panText, backgroundArea, juce::Justification::centred, false);
}

void PerformanceViewLookFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, 
	const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown
) {
	auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
	float cornerSize = 4.0f;

	juce::Colour bgColour = MyColours::unpressedBtn;

	getColourFromToggleState(button, bgColour);
	drawButton(g, bgColour, bounds, cornerSize, isButtonDown);
	handleMouseOverButton(isMouseOverButton, isButtonDown, g, bounds, cornerSize);
}

void PerformanceViewLookFeel::getColourFromToggleState(juce::Button& button, juce::Colour& colourToChange)
{
	if (button.getToggleState())
	{
		if (button.getName() == UIComponentNames::muteButton)
			colourToChange = MyColours::muteBtnPressed;
		else if (button.getName() == UIComponentNames::soloButton)
			colourToChange = MyColours::soloBtnPressed;
		else if (button.getName() == UIComponentNames::expandButton)
			colourToChange = MyColours::expandBtnPressed;
		else colourToChange = juce::Colours::orange;
	}
}

void PerformanceViewLookFeel::drawButton(juce::Graphics& g, const juce::Colour& bgColour, const juce::Rectangle<float>& bounds, float cornerSize, bool isButtonDown)
{
	auto buttonArea = bounds;

	if (isButtonDown)
	{
		buttonArea.translate(0.0f, 1.0f);
	}
	else
	{
		g.setColour(juce::Colours::black.withAlpha(0.4f));
		g.fillRoundedRectangle(buttonArea.translated(0.0f, 2.0f), cornerSize);
	}

	drawBtnBaseColour(isButtonDown, bgColour, buttonArea, g, cornerSize);
	drawBtnHardwareEdges(isButtonDown, g, buttonArea, cornerSize);
	drawBtnOutline(g, buttonArea, cornerSize);
}

void PerformanceViewLookFeel::drawBtnBaseColour(bool isButtonDown, const juce::Colour& bgColour, juce::Rectangle<float>& buttonArea, juce::Graphics& g, float cornerSize)
{
	juce::Colour baseColour = isButtonDown ? bgColour.darker(0.15f) : bgColour;
	juce::ColourGradient bgGradient(baseColour.brighter(0.05f), 0.0f, buttonArea.getY(),
		baseColour.darker(0.05f), 0.0f, buttonArea.getBottom(), false);
	g.setGradientFill(bgGradient);
	g.fillRoundedRectangle(buttonArea, cornerSize);
}

void PerformanceViewLookFeel::drawBtnHardwareEdges(bool isButtonDown, juce::Graphics& g, juce::Rectangle<float>& buttonArea, float cornerSize)
{
	if (!isButtonDown)
	{
		drawBtnTopRimHighlight(g, buttonArea, cornerSize);
		drawBtnBottomEdgeShadow(g, buttonArea, cornerSize);
	}
	else
	{
		drawBtnInsetShadow(g, buttonArea, cornerSize);
	}
}

void PerformanceViewLookFeel::drawBtnTopRimHighlight(juce::Graphics& g, juce::Rectangle<float>& buttonArea, float cornerSize)
{
	g.setColour(juce::Colours::white.withAlpha(0.12f));
	juce::Path topHighlight;
	topHighlight.addRoundedRectangle(buttonArea.reduced(1.0f), cornerSize - 1.0f);
	g.strokePath(topHighlight, juce::PathStrokeType(1.0f));
}

void PerformanceViewLookFeel::drawBtnBottomEdgeShadow(juce::Graphics& g, juce::Rectangle<float>& buttonArea, float cornerSize)
{
	g.setColour(juce::Colours::black.withAlpha(0.25f));
	g.drawHorizontalLine(static_cast<int>(buttonArea.getBottom() - 1.0f),
		buttonArea.getX() + cornerSize,
		buttonArea.getRight() - cornerSize);
}

void PerformanceViewLookFeel::drawBtnInsetShadow(juce::Graphics& g, juce::Rectangle<float>& buttonArea, float cornerSize)
{
	g.setColour(juce::Colours::black.withAlpha(0.6f));
	juce::Path innerShadow;
	innerShadow.addRoundedRectangle(buttonArea.reduced(0.5f), cornerSize);
	g.strokePath(innerShadow, juce::PathStrokeType(1.5f));
}

void PerformanceViewLookFeel::drawBtnOutline(juce::Graphics& g, const juce::Rectangle<float>& buttonArea, float cornerSize)
{
	g.setColour(juce::Colours::black.withAlpha(0.8f));
	g.drawRoundedRectangle(buttonArea, cornerSize, 1.0f);
}

void PerformanceViewLookFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, 
	bool isMouseOverButton, bool isButtonDown
) {
	g.setColour(button.getToggleState() ? juce::Colours::black : juce::Colours::white);

	float fontSize = juce::jmin(UISizeConstants::btnTextMaxFont, button.getWidth() * 0.6f);
	g.setFont(juce::Font(fontSize, juce::Font::bold));

	auto textBounds = button.getLocalBounds().toFloat();
	if (isButtonDown)
		textBounds.translate(0.0f, 1.0f);

	g.drawText(button.getButtonText(), textBounds, juce::Justification::centred, false);
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

void PerformanceViewLookFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
	g.fillAll(MyColours::background.brighter(0.05f));

	g.setColour(juce::Colours::black);
	g.drawRect(0, 0, width, height, 1);

	g.setColour(juce::Colours::white.withAlpha(0.05f));
	g.drawRect(1, 1, width - 2, height - 2, 1);
}

void PerformanceViewLookFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, 
	const bool isSeparator, const bool isActive, const bool isHighlighted, const bool isTicked, 
	const bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText, 
	const juce::Drawable* icon, const juce::Colour* textColourToUse
) {
	if (isSeparator)
	{
		drawSeparator(area, g);
		return;
	}

	if (isHighlighted && isActive)
	{
		g.setColour(MyColours::black.brighter(0.4f));
		g.fillRect(area.reduced(2, 1));
	}

	g.setColour(isActive ? juce::Colours::white : juce::Colours::grey.withAlpha(0.5f));
	g.setFont(juce::Font(UISizeConstants::popupMenuFont));

	auto r = area.reduced(12, 0);
	g.drawText(text, r, juce::Justification::centredLeft, true);

	if (hasSubMenu)
		drawSubMenu(area, g);
}

void PerformanceViewLookFeel::drawSeparator(const juce::Rectangle<int>& area, juce::Graphics& g)
{
	auto r = area.reduced(5, 0);
	r.removeFromTop(juce::roundToInt(((float)r.getHeight() * 0.5f) - 0.5f));
	g.setColour(juce::Colours::black.withAlpha(0.5f));
	g.fillRect(r.removeFromTop(1));
	g.setColour(juce::Colours::white.withAlpha(0.05f));
	g.fillRect(r.removeFromTop(1));
}

void PerformanceViewLookFeel::drawSubMenu(const juce::Rectangle<int>& area, juce::Graphics& g)
{
	auto localArea = area;
	auto arrowZone = localArea.removeFromRight(15).reduced(2);
	juce::Path path;
	path.addTriangle(arrowZone.getX(), arrowZone.getY() + 4.0f,
		arrowZone.getRight() - 4.0f, arrowZone.getCentreY(),
		arrowZone.getX(), arrowZone.getBottom() - 4.0f);
	g.fillPath(path);
}

void PerformanceViewLookFeel::getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int standardMenuItemHeight, int& idealWidth, int& idealHeight)
{
	if (isSeparator)
	{
		idealWidth = 50;
		idealHeight = 8;
	}
	else
	{
		juce::Font font(UISizeConstants::popupMenuFont);
		idealWidth = font.getStringWidth(text) + 40;
		idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : 28;
	}
}

void PerformanceViewLookFeel::updateGlobalTypography(float baselineSlotWidth)
{
	globalSharedFont = juce::jlimit(
		UISizeConstants::minFontSize,
		UISizeConstants::maxFontSize,
		baselineSlotWidth * UISizeConstants::fontSizeWidthRatio
	);
}
