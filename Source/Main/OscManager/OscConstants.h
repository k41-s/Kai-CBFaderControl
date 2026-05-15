#pragma once
#include <JuceHeader.h>

namespace OscAddressPatterns
{
    // The root address to listen to EVERYTHING from XPatch
    static const juce::String root = "/XP4/#";

    // Base prefixes
    static const juce::String prefix = "/XP4";
    static const juce::String fader = "/fader/";
    static const juce::String vca = "/vca/";

    // Parameter endpoints
    static const juce::String name = "/name";
    static const juce::String volume = "/volume";
    static const juce::String mute = "/mute";
    static const juce::String pan = "/pan";
    static const juce::String solo = "/solo";
}