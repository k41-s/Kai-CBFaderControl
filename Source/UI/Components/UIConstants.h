#pragma once
#include <JuceHeader.h>

namespace WindowSizeValues
{
	static inline const int minWidth = 750;
    static inline const int minHeight = 400;
    static inline const int defaultWidth = 900;
    static inline const int defaultHeight = 650;
    static inline const int maxWidth = 1920;
    static inline const int maxHeight = 1200;
}

namespace UIComponentNames
{
    static inline const juce::String muteButton = "MuteButton";
    static inline const juce::String soloButton = "SoloButton";
    static inline const juce::String activeToggle = "ActiveToggle";
}

namespace UIProperties
{
    static inline const juce::Identifier isHighRes = "isHighRes";
    static inline const juce::Identifier customColour = "customColour";
}

namespace UIButtonLabels
{
    static inline const juce::String mute = "M";
    static inline const juce::String solo = "S";
    static inline const juce::String soloInPlace = "SIP";
}