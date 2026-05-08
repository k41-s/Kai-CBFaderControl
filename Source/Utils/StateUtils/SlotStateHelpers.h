#pragma once
#include <JuceHeader.h>
#include "../../Main/SlotIDs.h"

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

    // SETTERS

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

    // SEMANTIC DOMAIN HELPERS (Slot Logic)

    static inline int getGroupId(const juce::ValueTree& state, int slotIdx)
    {
        return getIntProp(state, SlotIDs::groupId(slotIdx), 0);
    }

    static inline int getGroupRole(const juce::ValueTree& state, int slotIdx)
    {
        return getIntProp(state, SlotIDs::groupRole(slotIdx), 0);
    }

    static inline bool isSlotInGroup(const juce::ValueTree& state, int slotIdx)
    {
        return getGroupId(state, slotIdx) > 0;
    }

    static inline bool isGroupLeader(const juce::ValueTree& state, int slotIdx)
    {
        return isSlotInGroup(state, slotIdx) && getGroupRole(state, slotIdx) == 1;
    }

    static inline bool isStereoLinked(const juce::ValueTree& state, int slotIdx)
    {
        return getBoolProp(state, SlotIDs::isStereoLinked(slotIdx), false);
    }

    static inline bool isStereoMain(const juce::ValueTree& state, int slotIdx)
    {
        return getBoolProp(state, SlotIDs::isStereoMain(slotIdx), false);
    }

    static inline int getLinkedSlotId(const juce::ValueTree& state, int slotIdx)
    {
        return getIntProp(state, SlotIDs::linkedSlotId(slotIdx), -1);
    }

    static inline juce::String getSlotCustomName(const juce::ValueTree& state, int slotIdx)
    {
        return getStringProp(state, SlotIDs::slotName(slotIdx), "");
    }
}