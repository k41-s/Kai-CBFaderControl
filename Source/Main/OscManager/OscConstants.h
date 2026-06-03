#pragma once
#include <JuceHeader.h>

namespace OscConstants
{
    static const juce::String incomingPrefix = "XP4";
    static const juce::String outgoingPrefix = "XP4";

    static constexpr int fifoSize = 512;

    static constexpr int pingFrequency = 1000;
    static constexpr int timeoutThreshold = 3000;

    struct TargetTypes
    {
        static const juce::String fader() { return "fdr"; }
    };

    struct SystemCommands
    {
        static const juce::String ping() { return "ping"; }
        static const juce::String startup() { return "startup"; }
    };

    struct ParamTypes
    {
        static const juce::String name() { return "name"; }
        static const juce::String colour() { return "col"; }
        static const juce::String volume() { return "v"; }
        static const juce::String mute() { return "m"; }
        static const juce::String pan() { return "pan"; }
        static const juce::String solo() { return "solo"; }
        static const juce::String stereo() { return "stereo"; }
    };

    struct Parsing
    {
        static constexpr int standardPathDepth = 4;

        static constexpr float volumeThresholdDb = 0.01f;
        static constexpr float panThreshold = 0.01f;
    };
}