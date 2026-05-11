#pragma once
#include <JuceHeader.h>
#include "../../Components/UIConstants.h"

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
        bool isMouseOverButton, bool isButtonDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool isMouseOverButton, bool isButtonDown) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
        const bool isSeparator, const bool isActive, const bool isHighlighted, const bool isTicked,
        const bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
        const juce::Drawable* icon, const juce::Colour* textColourToUse) override;
    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
        int standardMenuItemHeight, int& idealWidth, int& idealHeight) override;

    void updateGlobalTypography(float baselineSlotWidth);
    float getStandardSharedFont() const { return globalSharedFont; }

private:
	//Fader Helpers
    void drawFader(int x, int y, int width, int height, juce::Graphics& g, juce::Slider& slider, float sliderPos);
    void drawFaderScale(juce::Graphics& g, juce::Slider& slider, const juce::Rectangle<float>& tickArea);
    void drawTickValues(std::vector<double>& tickValues, juce::Slider& slider, const juce::Rectangle<float>& tickArea, juce::Graphics& g);
    void drawTickLine(const juce::Rectangle<float>& tickArea, float lineLength, juce::Graphics& g, bool isZero, float y);
    void drawTickText(bool isZero, bool isInf, double val, const juce::Rectangle<float>& tickArea, float y, float lineLength, juce::Graphics& g);
    void drawFaderTrack(juce::Graphics& g, juce::Slider& slider, juce::Rectangle<float>& area);
	juce::Rectangle<float> getFaderCapBounds(juce::Rectangle<float>& area, float sliderPos);
    void setCapColour(juce::Slider& slider, juce::Colour& capColour, bool isHighResMode);
    void drawFaderCap(juce::Graphics& g, const juce::Colour& capColour, const juce::Rectangle<float>& capBounds);
    void drawFaderCapMainGradient(const juce::Rectangle<float>& capBounds, const juce::Colour& capColour, juce::Graphics& g);
    juce::ColourGradient getFaderCapMainGradient(const juce::Rectangle<float>& capBounds, const juce::Colour& capColour);
    void drawFaderCapRidges(const juce::Rectangle<float>& capBounds, juce::Graphics& g);
    void drawFaderCapSideShadow(const juce::Rectangle<float>& capBounds, juce::Graphics& g);
    void configSideShadow(juce::ColourGradient& sideShadow, const juce::Rectangle<float>& capBounds);
    void drawFaderCapOuterBorder(juce::Graphics& g, const juce::Rectangle<float>& capBounds);
    void drawIndicatorLine(juce::Graphics& g, juce::Rectangle<float>& capBounds, juce::Slider& slider);

	//Rotary Helpers
    void drawKnobBackground(juce::Graphics& g, float centreX, float radius, float centreY);
    void drawKnobDropShadow(juce::Graphics& g, float centreX, float centreY, float radius);
    void drawKnobGrip(juce::Graphics& g, float centreX, float centreY, float radius);
    void configAndDrawIndicatorPointer(float radius, float angle, float centreX, float centreY, juce::Graphics& g);
    void configIndicatorPointer(float radius, juce::Path& p, float angle, float centreX, float centreY);
    void drawIndicatorPointer(juce::Graphics& g, juce::Path& p);
    void drawOuterRing(juce::Graphics& g, float centreX, float radius, float centreY);
    void drawPanScale(juce::Graphics& g, float centreX, float centreY, float radius, float startAngle, float endAngle);
    void drawPanLabels(juce::Graphics& g, float centreX, float scaleRadius, float centreY, float startAngle, float endAngle);
    void drawPanTicks(juce::Graphics& g, float radius, float centreX, float centreY, float startAngle, float endAngle);
    void drawPanValueText(juce::Graphics& g, float sliderPos, juce::Rectangle<float>& textArea);

    //Button Helpers
    void getColourFromToggleState(juce::Button& button, juce::Colour& bgColour);
    void drawButton(juce::Graphics& g, const juce::Colour& bgColour, const juce::Rectangle<float>& bounds, float cornerSize, bool isButtonDown);
    void drawBtnBaseColour(bool isButtonDown, const juce::Colour& bgColour, juce::Rectangle<float>& buttonArea, juce::Graphics& g, float cornerSize);
    void drawBtnHardwareEdges(bool isButtonDown, juce::Graphics& g, juce::Rectangle<float>& buttonArea, float cornerSize);
    void drawBtnTopRimHighlight(juce::Graphics& g, juce::Rectangle<float>& buttonArea, float cornerSize);
    void drawBtnBottomEdgeShadow(juce::Graphics& g, juce::Rectangle<float>& buttonArea, float cornerSize);
    void drawBtnInsetShadow(juce::Graphics& g, juce::Rectangle<float>& buttonArea, float cornerSize);
    void drawBtnOutline(juce::Graphics& g, const juce::Rectangle<float>& buttonArea, float cornerSize);
    void handleMouseOverButton(bool isMouseOverButton, bool isButtonDown,
        juce::Graphics& g, juce::Rectangle<float>& area, float cornerSize);

	// PopupMenu Helpers
    void drawSeparator(const juce::Rectangle<int>& area, juce::Graphics& g);
    void drawSubMenu(const juce::Rectangle<int>& area, juce::Graphics& g);

    float globalSharedFont = UISizeConstants::standardFontSize;
    double inf = PluginConstants::infCutoff;
};