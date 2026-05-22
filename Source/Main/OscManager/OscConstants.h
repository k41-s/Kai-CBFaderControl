#pragma once
#include <JuceHeader.h>

namespace OscConstants
{
    static const juce::String incomingPrefix = "/XP4";
    static const juce::String outgoingPrefix = "/Plugin";

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
        static constexpr int standardPathDepth = 4;

        static constexpr float volumeThresholdDb = 0.01f;
        static constexpr float panThreshold = 0.01f;
    };
}