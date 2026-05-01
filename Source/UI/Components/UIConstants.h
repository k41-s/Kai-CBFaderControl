#pragma once
#include <JuceHeader.h>

namespace UIProperties
{
    static inline const juce::Identifier isHighRes = "isHighRes";
    static inline const juce::Identifier customColour = "customColour";

    static constexpr bool defaultHighRes = true;
}

namespace WindowSizeValues
{
	static inline const int minWidth = 750;
    static inline const int minHeight = 550;
    static inline const int defaultWidth = 900;
    static inline const int defaultHeight = 650;
    static inline const int maxWidth = 1920;
    static inline const int maxHeight = 1200;
}

namespace UIButtonLabels
{
    static inline const juce::String mute = "M";
    static inline const juce::String solo = "S";
    static inline const juce::String soloInPlace = "SIP";
}

namespace UIComponentNames
{
    static inline const juce::String muteButton = "MuteButton";
    static inline const juce::String soloButton = "SoloButton";
    static inline const juce::String activeToggle = "ActiveToggle";
    static inline const juce::String expandButton = "ExpandButton";
}

namespace SlotSizeValues
{
    static inline const float stereoSlotFlexGrowFactor = 1.75f;
    static inline const float vcaSlotFlexGrowFactor = 1.4f;
    static inline const float monoSlotFlexGrowFactor = 1.0f;

    static inline const float stereoSlotMaxWidth = 160.0f;
    static inline const float vcaSlotMaxWidth = 140.0f;
    static inline const float monoSlotMaxWidth = 120.0f;

    static inline const float stereoSlotTargetWidth = 100.0f;
    static inline const float vcaSlotTargetWidth = 80.0f;
    static inline const float monoSlotTargetWidth = 60.0f;

    static inline const int targetBtnWidth = 45;
}