#pragma once
#include <JuceHeader.h>

namespace SlotStateHelpers
{
    // =========================================================================
    // GETTERS
    // =========================================================================

    static inline int getIntProp(const juce::ValueTree& state, const juce::String& paramId, int defaultVal = 0)
    {
        return state.getProperty(juce::Identifier(paramId), defaultVal);
    }

    static inline bool getBoolProp(const juce::ValueTree& state, const juce::String& paramId, bool defaultVal = false)
    {
        return state.getProperty(juce::Identifier(paramId), defaultVal);
    }

    static inline juce::String getStringProp(const juce::ValueTree& state, const juce::String& paramId, const juce::String& defaultVal = "")
    {
        return state.getProperty(juce::Identifier(paramId), defaultVal).toString();
    }

    // =========================================================================
    // SETTERS
    // =========================================================================

    static inline void setIntProp(juce::ValueTree& state, const juce::String& paramId, int value, juce::UndoManager* undoManager = nullptr)
    {
        state.setProperty(juce::Identifier(paramId), value, undoManager);
    }

    static inline void setBoolProp(juce::ValueTree& state, const juce::String& paramId, bool value, juce::UndoManager* undoManager = nullptr)
    {
        state.setProperty(juce::Identifier(paramId), value, undoManager);
    }

    static inline void setStringProp(juce::ValueTree& state, const juce::String& paramId, const juce::String& value, juce::UndoManager* undoManager = nullptr)
    {
        state.setProperty(juce::Identifier(paramId), value, undoManager);
    }

    static inline void removeProp(juce::ValueTree& state, const juce::String& paramId, juce::UndoManager* undoManager = nullptr)
    {
        state.removeProperty(juce::Identifier(paramId), undoManager);
    }
}