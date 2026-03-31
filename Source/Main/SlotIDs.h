#pragma once
#include <JuceHeader.h>

struct SlotIDs
{
    static juce::String isActive(int i) { return "isActive_" + juce::String(i); }
    static juce::String volume(int i) { return "volume_" + juce::String(i); }
    static juce::String mute(int i) { return "mute_" + juce::String(i); }
    static juce::String pan(int i) { return "pan_" + juce::String(i); }
    static juce::String solo(int i) { return "solo_" + juce::String(i); }
    static juce::String soloSafe(int i) { return "soloSafe_" + juce::String(i); }

    static juce::Identifier targetIP() { return "targetIP"; }
    static juce::Identifier incomingPort() { return "incomingPort"; }
    static juce::Identifier outgoingPort() { return "outgoingPort"; }
};