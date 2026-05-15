#pragma once
#include <JuceHeader.h>

namespace OscConstants
{
    static const juce::String prefix = "/XP4";
    static constexpr int fifoSize = 512;

    struct TargetTypes
    {
        static const juce::String fader() { return "fader"; }
        static const juce::String vca() { return "vca"; }
    };

    struct ParamTypes
    {
        static const juce::String name() { return "name"; }
        static const juce::String volume() { return "volume"; }
        static const juce::String mute() { return "mute"; }
        static const juce::String pan() { return "pan"; }
        static const juce::String solo() { return "solo"; }
    };

    struct Parsing
    {
        static constexpr int standardPathDepth = 5;

        static constexpr float volumeThresholdDb = 0.01f;
        static constexpr float panThreshold = 0.01f;
    };
}