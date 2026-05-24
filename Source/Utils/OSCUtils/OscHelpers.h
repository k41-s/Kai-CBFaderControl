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

    static inline bool hasFloatChanged(const juce::String& paramType, float a, float b)
    {
        if (paramType == OscConstants::ParamTypes::pan())
            return panRawChanged(a, b);
        else
            return volumeRawChanged(a, b);
    }

    static inline juce::String buildAddress(const juce::String& target, int id, const juce::String& param)
    {
        return OscConstants::outgoingPrefix + "/" + target + "/" + juce::String(id) + "/" + param;
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

    static inline bool shouldSendPing(juce::uint32 currentTime, juce::uint32 lastMessageReceivedTime, juce::uint32 lastPingSentTime)
    {
        return (currentTime - lastMessageReceivedTime >= OscConstants::pingFrequency) &&
            (currentTime - lastPingSentTime >= OscConstants::pingFrequency);
    }

    template <typename T>
    static inline bool isDuplicateValue(const std::unordered_map<juce::String, T>& map, const juce::String& key, const T& newValue)
    {
        auto it = map.find(key);
        return (it != map.end() && it->second == newValue);
    }

    static inline bool isDuplicateFloat(const std::unordered_map<juce::String, float>& map, const juce::String& key, float newValue, const juce::String& paramType)
    {
        auto it = map.find(key);
        return (it != map.end() && !OscHelpers::hasFloatChanged(paramType, it->second, newValue));
    }
}