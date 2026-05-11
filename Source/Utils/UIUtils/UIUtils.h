#pragma once
#include <JuceHeader.h>
#include "../../UI/CustomLookAndFeel/MyColours.h"
#include "../../UI/Components/UIConstants.h"

namespace UIUtils
{
    /*
      Configures a standard dark-background label for displaying values and units,
      adds it to the parent component, and optionally sets initial text.
     */
    static inline void setupValueBoxLabel(juce::Component& parent, juce::Label& label, juce::Justification justification, const juce::String& text = "")
    {
        parent.addAndMakeVisible(label);
        label.setJustificationType(justification);
        label.setColour(juce::Label::backgroundColourId, MyColours::valueBackground);
        label.setColour(juce::Label::textColourId, MyColours::white);
        label.setBorderSize(juce::BorderSize<int>(0, 1, 0, 0));

        if (text.isNotEmpty()) {
            label.setText(text, juce::dontSendNotification);
        }
    }

    static inline juce::String getValueText(float val, bool isFineMode)
    {
        juce::String text;

        if (val <= PluginConstants::infCutoff)
            text = UIStringConstants::inf;
        else if (isFineMode)
            text = juce::String(val, 2);
        else
            text = juce::String(juce::roundToInt(val));

        return text;
    }

    /*
      Dynamically calculates and sets the font size for a label 
      based on the current width of the component
	 */
    static inline void setSharedFont(juce::Font& sharedFont, int currentWidth)
    {
        float sharedFontSize = juce::jlimit(
            UISizeConstants::minFontSize,
            UISizeConstants::maxFontSize,
            (float)currentWidth * UISizeConstants::fontSizeWidthRatio
        );
        sharedFont = juce::Font(sharedFontSize);
    }

    /*
      Converts a normalized pan slider position (0.0f to 1.0f) into a
      formatted string (e.g., "C", "L70", "R45").
    */
    static inline juce::String getPanValueText(float sliderPos)
    {
        if (std::abs(sliderPos - 0.5f) < 0.01f)
            return "C";

        int pct = juce::roundToInt(std::abs(sliderPos - 0.5f) * 200.0f);
        juce::String side = sliderPos < 0.5f ? "L" : "R";

        if (pct == 100)
            return side;

        return side + juce::String(pct);
    }
}