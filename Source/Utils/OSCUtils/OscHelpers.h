#pragma once
#include <JuceHeader.h>
#include "../../Main/OscManager/OscConstants.h"

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