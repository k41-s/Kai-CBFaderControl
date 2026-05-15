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

namespace OscHelpers
{
    static inline bool volumeRawChanged(float currentRaw, float incomingRaw)
    {
        return std::abs(currentRaw - incomingRaw) > OscConstants::Parsing::volumeThresholdDb;
    }

    static inline bool panRawChanged(float currentRaw, float incomingRaw)
    {
        return std::abs(currentRaw - incomingRaw) > OscConstants::Parsing::panThreshold;
    }

    static inline juce::String buildAddress(const juce::String& target, int id, const juce::String& param)
    {
        return OscConstants::prefix + "/" + target + "/" + juce::String(id) + "/" + param;
    }

    static inline bool isValidIntMessage(const juce::OSCMessage& message)
    {
        return message.size() > 0 && message[0].isInt32();
	}

    static inline bool isValidFloatMessage(const juce::OSCMessage& message)
    {
        return message.size() > 0 && message[0].isFloat32();
	}

    static inline bool isValidStringMessage(const juce::OSCMessage& message)
    {
        return message.size() > 0 && message[0].isString();
	}
}