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

        if (val <= -95.75f)
            text = UIStringConstants::inf;
        else if (isFineMode)
            text = juce::String(val, 2);
        else
            text = juce::String(juce::roundToInt(val));

        return text;
    }

    static inline void setSharedFont(juce::Font& sharedFont, int currentWidth)
    {
        float sharedFontSize = juce::jlimit(
            UISizeConstants::minFontSize,
            UISizeConstants::maxFontSize,
            (float)currentWidth * UISizeConstants::fontSizeWidthRatio
        );
        sharedFont = juce::Font(sharedFontSize);
    }
}