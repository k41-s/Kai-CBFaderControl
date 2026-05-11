#pragma once
#include <JuceHeader.h>

namespace UIProperties
{
    static inline const juce::Identifier isHighRes = "isHighRes";
    static inline const juce::Identifier customColour = "customColour";
    static inline const juce::Identifier indicatorColour = "indicatorColour";

    static constexpr bool defaultHighRes = true;
}

namespace WindowSizeValues
{
    static inline const int absolutePerfMinWidth = 350;
    static inline const int perfMinHeight = 600;
	static inline const int minWidth = 550;
    static inline const int minHeight = 500;
    static inline const int defaultWidth = 900;
    static inline const int defaultHeight = 650;
    static inline const int maxWidth = 1920;
    static inline const int maxHeight = 1200;
}

namespace UILayoutPercentages
{
    static constexpr float panHeightPct = 0.10f;
    static constexpr float faderCapWidthPct = 0.70f;
    static constexpr float faderCapMaxHeightPct = 0.15f;
    static constexpr float faderTrackWidthPct = 0.12f;

    static constexpr float tickAreaWidthPct = 0.30f;
    static constexpr float rotaryTickAreaWidthPct = 0.35f;

    static constexpr float knobDiameterPct = 0.95f;
    static constexpr float knobInnerRadiusPct = 0.82f;
    static constexpr float knobPointerLengthPct = 0.65f;
}

namespace UISizeConstants
{
    // --- FONT LIMITS ---
	static inline const float minFontSize = 9.0f;
	static inline const float standardFontSize = 12.0f;
	static inline const float maxFontSize = 15.0f;
	static inline const float fontSizeWidthRatio = 0.23f;

    static constexpr float faderScaleMinFont = 8.5f;
    static constexpr float faderScaleMaxFont = 13.0f;
    static constexpr float panTextMinFont = 7.0f;
    static constexpr float panTextMaxFont = 9.5f;
    static constexpr float popupMenuFont = 14.0f;
    static constexpr float btnTextMaxFont = 14.0f;

    // --- LAYOUT PIXELS ---
    static constexpr int slotPadding = 5;
    static constexpr int slotBottomAreaHeight = 25;
    static constexpr int slotBtnHeight = 30;
    static constexpr int minPanHeight = 35;

    static constexpr int setupLabelHeight = 30;
    static constexpr int setupEditorHeight = 40;
    static constexpr int setupSectionPadding = 10;

    // LookAndFeel Constraints
    static constexpr float faderCapMinWidth = 13.0f;
    static constexpr float faderCapMaxWidth = 24.0f;
    static constexpr float faderCapMinHeight = 16.0f;
    static constexpr float faderTrackMinWidth = 2.0f;
    static constexpr float faderTrackMaxWidth = 6.0f;
    static constexpr float tickAreaMinWidth = 16.0f;
    static constexpr float tickAreaMaxWidth = 30.0f;
    static constexpr float knobMinDiameter = 24.0f;
    static constexpr float knobMaxDiameter = 36.0f;
}

namespace UIButtonLabels
{
    static inline const juce::String mute = "M";
    static inline const juce::String solo = "S";
    static inline const juce::String soloInPlace = "SIP";
    static inline const juce::String expand = "EXP";
}

namespace UIGroupLabelPrefixes
{
    static inline const juce::String vca = "VCA ";
    static inline const juce::String leader = "LDR ";
    static inline const juce::String group = "GRP ";
}

namespace UIComponentNames
{
    static inline const juce::String muteButton = "MuteButton";
    static inline const juce::String soloButton = "SoloButton";
    static inline const juce::String activeToggle = "ActiveToggle";
    static inline const juce::String expandButton = "ExpandButton";
}

namespace UIStringConstants
{
	static inline const juce::String inf = "-inf";
    static inline const juce::String unitDb = "dB";
    static inline const juce::String maxWidthValueStr = "-88.8 ";
    static inline const juce::String infSymbol = juce::String::charToString(0x221E);
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

    static inline const float stereoSlotMinWidth = 70.0f;
    static inline const float vcaSlotMinWidth = 55.0f;
    static inline const float monoSlotMinWidth = 40.0f;

    static inline const int targetBtnWidth = 45;
    static inline const int targetFaderWidth = 60;
}

namespace PluginConstants
{
    static constexpr int numSlots = 32;
    static constexpr int numVcas = 8;
    static constexpr int numGroups = 8;

    static constexpr float volumeMax = 22.0f;
    static constexpr float volumeMin = -96.0f;
    static constexpr float infCutoff = -95.75f;
    static constexpr float fineRes = 0.25f;
    static constexpr float coarseRes = 1.0f;
    
	static inline const int maxSlotNameLength = 5;
	static inline const int maxGroupNameLength = maxSlotNameLength; // for now unless they need to be different
}