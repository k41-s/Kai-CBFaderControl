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

    // =========================================================================
    // SEMANTIC DOMAIN GETTERS (Slot Logic)
    // =========================================================================

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

    static inline int getGroupColour(const juce::ValueTree& state, int grpId)
    {
        return getIntProp(state, SlotIDs::groupColour(grpId), 0);
    }

    static inline juce::String getVcaName(const juce::ValueTree& state, int vcaIdx)
    {
        return getStringProp(state, SlotIDs::vcaName(vcaIdx), "");
    }

    static inline juce::String getTargetIP(const juce::ValueTree& state)
    {
        return getStringProp(state, SlotIDs::targetIP().toString(), "127.0.0.1");
    }

    static inline int getIncomingPort(const juce::ValueTree& state)
    {
        return getIntProp(state, SlotIDs::incomingPort().toString(), 8000);
    }

    static inline int getOutgoingPort(const juce::ValueTree& state)
    {
        return getIntProp(state, SlotIDs::outgoingPort().toString(), 8001);
    }

    // =========================================================================
    // SEMANTIC DOMAIN SETTERS (Slot Logic)
    // =========================================================================

    static inline void setGroupId(juce::ValueTree& state, int slotIdx, int groupId, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::groupId(slotIdx), groupId, undoManager);
    }

    static inline void setGroupRole(juce::ValueTree& state, int slotIdx, int role, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::groupRole(slotIdx), role, undoManager);
    }

    static inline void setStereoLinked(juce::ValueTree& state, int slotIdx, bool isLinked, juce::UndoManager* undoManager = nullptr)
    {
        setBoolProp(state, SlotIDs::isStereoLinked(slotIdx), isLinked, undoManager);
    }

    static inline void setStereoMain(juce::ValueTree& state, int slotIdx, bool isMain, juce::UndoManager* undoManager = nullptr)
    {
        setBoolProp(state, SlotIDs::isStereoMain(slotIdx), isMain, undoManager);
    }

    static inline void setLinkedSlotId(juce::ValueTree& state, int slotIdx, int linkedIdx, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::linkedSlotId(slotIdx), linkedIdx, undoManager);
    }

    static inline void setSlotCustomName(juce::ValueTree& state, int slotIdx, const juce::String& name, juce::UndoManager* undoManager = nullptr)
    {
        if (name.isEmpty())
            removeProp(state, SlotIDs::slotName(slotIdx), undoManager);
        else
            setStringProp(state, SlotIDs::slotName(slotIdx), name, undoManager);
    }

    static inline void setGroupColour(juce::ValueTree& state, int grpId, int colourIdx, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::groupColour(grpId), colourIdx, undoManager);
    }

    static inline void setVcaName(juce::ValueTree& state, int vcaIdx, const juce::String& name, juce::UndoManager* undoManager = nullptr)
    {
        if (name.isEmpty())
            removeProp(state, SlotIDs::vcaName(vcaIdx), undoManager);
        else
            setStringProp(state, SlotIDs::vcaName(vcaIdx), name, undoManager);
    }

    static inline void setTargetIP(juce::ValueTree& state, const juce::String& ip, juce::UndoManager* undoManager = nullptr)
    {
        setStringProp(state, SlotIDs::targetIP().toString(), ip, undoManager);
    }

    static inline void setIncomingPort(juce::ValueTree& state, int port, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::incomingPort().toString(), port, undoManager);
    }

    static inline void setOutgoingPort(juce::ValueTree& state, int port, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::outgoingPort().toString(), port, undoManager);
    }

    // =========================================================================
    // APVTS PARAMETER HELPERS (Audio Parameters)
    // =========================================================================

    static inline float getRawParamValue(const juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
    {
        if (auto* param = apvts.getRawParameterValue(paramId))
            return param->load();
        return 0.0f;
    }

    static inline bool isVcaEnabled(const juce::AudioProcessorValueTreeState& apvts, int grpId)
    {
        return getRawParamValue(apvts, SlotIDs::vcaEnabled(grpId)) > 0.5f;
    }

    static inline bool isVcaExpanded(const juce::AudioProcessorValueTreeState& apvts, int grpId)
    {
        return getRawParamValue(apvts, SlotIDs::isVcaExpanded(grpId)) > 0.5f;
    }

    static inline bool isSlotActive(const juce::AudioProcessorValueTreeState& apvts, int slotIdx)
    {
        return getRawParamValue(apvts, SlotIDs::isActive(slotIdx)) > 0.5f;
    }

    static inline void setParamNormalized(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, float normalizedValue)
    {
        if (auto* param = apvts.getParameter(paramId))
            param->setValueNotifyingHost(normalizedValue);
    }

    static inline void setParamUnnormalized(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, float unnormalizedValue)
    {
        if (auto* param = apvts.getParameter(paramId))
            param->setValueNotifyingHost(param->convertTo0to1(unnormalizedValue));
    }
}