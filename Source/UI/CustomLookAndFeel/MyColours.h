#pragma once
#include <JuceHeader.h>

namespace MyColours
{
    const juce::Colour cbBlue = juce::Colour::fromRGB(28, 45, 135);

    const juce::Colour background = juce::Colour(0xFF252422);
    const juce::Colour txtEditorBackground = juce::Colour::fromRGB(254, 248, 231);

    const juce::Colour faderTrack = juce::Colour::fromRGB(20, 20, 20);
    const juce::Colour faderCap = juce::Colours::white.darker(0.1f);

    const juce::Colour rotarySliderBackground = juce::Colour(0xFF313131);

    const juce::Colour knobBaseGradientStart = juce::Colour(0xFF3A3A3A);
    const juce::Colour knobBaseGradientEnd = juce::Colour(0xFF151515);

    const juce::Colour knobInnerGradientStart = juce::Colour(0xFF282828);
    const juce::Colour knobInnerGradientEnd = juce::Colour(0xFF1A1A1A);

    const juce::Colour black = juce::Colours::black.withAlpha(0.9f);
    const juce::Colour white = juce::Colours::white.withAlpha(0.9f);

    const juce::Colour valueBackground = juce::Colour(0xFF171717);
    
    const juce::Colour unpressedBtn = juce::Colour(0xFF2A2A2A);
    const juce::Colour mouseOverButton = juce::Colours::white.withAlpha(0.05f);
	const juce::Colour muteBtnPressed = juce::Colour(0xFFD24242).withAlpha(0.8f);
	const juce::Colour soloBtnPressed = juce::Colour(0xFFF4EB66).withAlpha(0.8f);
	const juce::Colour expandBtnPressed = juce::Colour(0xFFBAE361).withAlpha(0.8f); // pastel green

    const juce::Colour setupViewLassoBackground = juce::Colours::grey.withAlpha(0.15f);
    const juce::Colour setupViewLassoOutline = juce::Colours::grey.withAlpha(0.6f);
}

namespace GroupColours
{
    const juce::Colour cyan = juce::Colour(0xFF56FFF9);
	const juce::Colour red = juce::Colour(0xFFFF0000).brighter(0.3f);
    const juce::Colour blue = juce::Colour(0xFF0A2AFF).brighter(0.5f);
    const juce::Colour pink = juce::Colour(0xFFFE019A);
    const juce::Colour green = juce::Colour(0xFF21FA90);
    const juce::Colour purple = juce::Colour(0xFF9A00D7).brighter(0.4f);
    const juce::Colour orange = juce::Colour(0xFFFF8000);
    const juce::Colour yellow = juce::Colour(0xFFFFD500);

    static constexpr int numColours = 9;

    static inline const juce::Colour palette[numColours] = {
        cyan, 
        red,
        blue, 
        pink, 
        green, 
        purple, 
        orange, 
        yellow,
        MyColours::white
    };

    static inline const juce::String names[numColours] = {
        "Cyan", 
        "Red", 
        "Blue", 
        "Pink", 
        "Green", 
        "Purple", 
        "Orange", 
        "Yellow",
        "White"
    };
}

namespace LinkColours
{
    static constexpr int numColours = 24;
    static inline const juce::Colour palette[numColours] = {
        juce::Colour(0xFFFF5252), juce::Colour(0xFFFF4081), juce::Colour(0xFFE040FB),
        juce::Colour(0xFF7C4DFF), juce::Colour(0xFF536DFE), juce::Colour(0xFF448AFF),
        juce::Colour(0xFF40C4FF), juce::Colour(0xFF18FFFF), juce::Colour(0xFF64FFDA),
        juce::Colour(0xFF69F0AE), juce::Colour(0xFFB2FF59), juce::Colour(0xFFEEFF41),
        juce::Colour(0xFFFFFF00), juce::Colour(0xFFFFD740), juce::Colour(0xFFFFAB40),
        juce::Colour(0xFFFF6E40), juce::Colour(0xFFA1887F), juce::Colour(0xFFE0E0E0),
        juce::Colour(0xFF90A4AE), juce::Colour(0xFF00BFA5), juce::Colour(0xFFC0CA33),
        juce::Colour(0xFFFDD835), juce::Colour(0xFF8D6E63), juce::Colour(0xFF26A69A)
    };
}