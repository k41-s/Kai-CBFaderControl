#pragma once
#include <JuceHeader.h>

class PerformanceViewLookFeel : public juce::LookAndFeel_V4
{
public:
	PerformanceViewLookFeel();

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, const float startAngle, const float endAngle,
        juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool isMouseOverButton, bool isButtonDown) override;
private:
    void drawFader(int x, int y, int width, int height, juce::Graphics& g, juce::Slider& slider, float sliderPos);
    void drawFaderTrack(juce::Graphics& g, juce::Slider& slider, juce::Rectangle<float>& area);
    void setCapColour(juce::Slider& slider, juce::Colour& capColour, bool isHighResMode);
    void drawFaderCap(juce::Graphics& g, const juce::Colour& capColour, const juce::Rectangle<float>& capBounds);
    void drawIndicatorLine(juce::Graphics& g, juce::Rectangle<float>& capBounds);

    void drawKnobBackground(juce::Graphics& g, float centreX, float radius, float centreY);
    void configAndDrawIndicatorPointer(float radius, float angle, float centreX, float centreY, juce::Graphics& g);
    void configIndicatorPointer(float radius, juce::Path& p, float angle, float centreX, float centreY);
    void drawIndicatorPointer(juce::Graphics& g, juce::Path& p);
    void drawOuterRing(juce::Graphics& g, float centreX, float radius, float centreY);

    void getColourFromToggleState(juce::Button& button, juce::Colour& bgColour);
    void drawButton(juce::Graphics& g, const juce::Colour& bgColour, const juce::Rectangle<float>& bounds);
};