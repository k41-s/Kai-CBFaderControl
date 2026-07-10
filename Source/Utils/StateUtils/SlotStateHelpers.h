#pragma once
#include <JuceHeader.h>
#include "../../Main/SlotIDs.h"
#include "../../UI/Components/UIConstants.h"
#include "../../UI/CustomLookAndFeel/MyColours.h"
#include "../Enums/GroupRole.h"
#include "../../Main/PresetManager/PresetConstants.h"

namespace SlotStateHelpers
{
    namespace detail
    {
        static inline int findManualGroupColour(const juce::ValueTree& state, int targetGrpId, std::array<bool, GroupColours::numColours>& isColourUsed);
        static inline int resolveAutoGroupColour(const juce::ValueTree& state, int targetGrpId, std::array<bool, GroupColours::numColours>& isColourUsed);
    }
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

    static inline bool isValidGroup(int grpId)
    {
        return grpId > 0;
    }

    static inline int getGroupId(const juce::ValueTree& state, int slotIdx)
    {
        return getIntProp(state, SlotIDs::groupId(slotIdx), 0);
    }

    static inline GroupRole getGroupRole(const juce::ValueTree& state, int slotIdx)
    {
        return static_cast<GroupRole>(getIntProp(state, SlotIDs::groupRole(slotIdx), 0));
    }

    static inline bool isSlotInGroup(const juce::ValueTree& state, int slotIdx)
    {
        return getGroupId(state, slotIdx) > 0;
    }

    static inline bool isGroupLeader(const juce::ValueTree& state, int slotIdx)
    {
        return isSlotInGroup(state, slotIdx) && getGroupRole(state, slotIdx) == GroupRole::Leader;
    }

    static inline bool isStereoLinked(const juce::ValueTree& state, int slotIdx)
    {
        return getBoolProp(state, SlotIDs::isStereoLinked(slotIdx), false);
    }

    static inline bool isStereoMain(const juce::ValueTree& state, int slotIdx)
    {
        return getBoolProp(state, SlotIDs::isStereoMain(slotIdx), false);
    }

    static inline bool isXpStereo(const juce::ValueTree& state, int slotIdx)
    {
        return getBoolProp(state, SlotIDs::xpStereo(slotIdx), false);
    }

    static inline int getLinkedSlotId(const juce::ValueTree& state, int slotIdx)
    {
        return getIntProp(state, SlotIDs::linkedSlotId(slotIdx), -1);
    }

    static inline bool isSipMuted(const juce::ValueTree& state, int slotIdx)
    {
        return getBoolProp(state, SlotIDs::sipMuted(slotIdx), false);
    }

    static inline juce::String getSlotCustomName(const juce::ValueTree& state, int slotIdx)
    {
        return getStringProp(state, SlotIDs::slotName(slotIdx), "");
    }

    static inline juce::String getSlotColour(const juce::ValueTree& state, int slotIdx)
    {
        return getStringProp(state, SlotIDs::slotColour(slotIdx), "");
    }

    static inline int getGroupColour(const juce::ValueTree& state, int grpId)
    {
        int whiteIndex = GroupColours::numColours - 1;

        if (grpId < 1 || grpId > PluginConstants::numGroups)
            return whiteIndex;

        std::array<bool, GroupColours::numColours> isColourUsed = { false };

        int manualColour = detail::findManualGroupColour(state, grpId, isColourUsed);
        if (manualColour != -1)
            return manualColour;

        return detail::resolveAutoGroupColour(state, grpId, isColourUsed);
    }

    static inline bool isGroupColourClaimed(const juce::ValueTree& state, int colourIdx, int ignoreGroupId = -1)
    {
        for (int i = 1; i <= PluginConstants::numGroups; ++i)
        {
            if (i == ignoreGroupId)
                continue;

            juce::Identifier propId = SlotIDs::groupColour(i);

            if (state.hasProperty(propId))
            {
                int manualColourIndex = static_cast<int>(state.getProperty(propId));
                if (manualColourIndex == colourIdx)
                {
                    return true;
                }
            }
        }
        return false;
    }

    static inline void clearGroupColour(juce::ValueTree& state, int grpId, juce::UndoManager* undoManager = nullptr)
    {
        removeProp(state, SlotIDs::groupColour(grpId), undoManager);
    }

    static inline juce::String getVcaName(const juce::ValueTree& state, int vcaIdx)
    {
        return getStringProp(state, SlotIDs::vcaName(vcaIdx), "");
    }

    static inline juce::String getTargetIP(const juce::ValueTree& state)
    {
        return getStringProp(state, SlotIDs::targetIP().toString(), NetworkConstants::defaultTargetIp);
    }

    static inline int getIncomingPort(const juce::ValueTree& state)
    {
        return getIntProp(state, SlotIDs::incomingPort().toString(), NetworkConstants::defaultIncomingPort);
    }

    static inline int getOutgoingPort(const juce::ValueTree& state)
    {
        return getIntProp(state, SlotIDs::outgoingPort().toString(), NetworkConstants::defaultOutgoingPort);
    }

    static inline int isPluginConnected(const juce::ValueTree& state)
    {
        return getBoolProp(state, SlotIDs::isConnected().toString(), false);
    }

    static inline bool isLinkPolarityInverse(const juce::ValueTree& state, int slotIdx) {
        return getBoolProp(state, SlotIDs::linkPolarityInverse(slotIdx), false);
    }

    static inline bool isLinkMaskVolume(const juce::ValueTree& state, int slotIdx) {
        return getBoolProp(state, SlotIDs::linkMaskVolume(slotIdx), true);
    }

    static inline bool isLinkMaskMute(const juce::ValueTree& state, int slotIdx) {
        return getBoolProp(state, SlotIDs::linkMaskMute(slotIdx), true);
    }

    static inline bool isLinkMaskSolo(const juce::ValueTree& state, int slotIdx) {
        return getBoolProp(state, SlotIDs::linkMaskSolo(slotIdx), true);
    }

    static inline bool isLinkMaskPan(const juce::ValueTree& state, int slotIdx) {
        return getBoolProp(state, SlotIDs::linkMaskPan(slotIdx), true);
    }

    static inline int getCustomLinkedId(const juce::ValueTree& state, int slotIdx)
    {
        return getIntProp(state, SlotIDs::customLinkedId(slotIdx), 0);
    }

    static inline bool getCustomLinkedIsVca(const juce::ValueTree& state, int slotIdx)
    {
        return getBoolProp(state, SlotIDs::customLinkedIsVca(slotIdx), false);
    }

    static inline int getLinkColourIndex(const juce::ValueTree& state, int slotIdx) 
    {
        return getIntProp(state, SlotIDs::linkColourIndex(slotIdx), -1);
    }

    // =========================================================================
    // SEMANTIC DOMAIN SETTERS (Slot Logic)
    // =========================================================================

    static inline void setGroupId(juce::ValueTree& state, int slotIdx, int groupId, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::groupId(slotIdx), groupId, undoManager);
    }

    static inline void setGroupRole(juce::ValueTree& state, int slotIdx, GroupRole role, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::groupRole(slotIdx), static_cast<int>(role), undoManager);
    }

    static inline void setStereoLinked(juce::ValueTree& state, int slotIdx, bool isLinked, juce::UndoManager* undoManager = nullptr)
    {
        setBoolProp(state, SlotIDs::isStereoLinked(slotIdx), isLinked, undoManager);
    }

    static inline void setStereoMain(juce::ValueTree& state, int slotIdx, bool isMain, juce::UndoManager* undoManager = nullptr)
    {
        setBoolProp(state, SlotIDs::isStereoMain(slotIdx), isMain, undoManager);
    }

    static inline void setXpStereo(juce::ValueTree& state, int slotIdx, bool isStereo, juce::UndoManager* undoManager = nullptr)
    {
        setBoolProp(state, SlotIDs::xpStereo(slotIdx), isStereo, undoManager);
    }

    static inline void setLinkedSlotId(juce::ValueTree& state, int slotIdx, int linkedIdx, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::linkedSlotId(slotIdx), linkedIdx, undoManager);
    }

    static inline void setSipMuted(juce::ValueTree& state, int slotIdx, bool isMuted, juce::UndoManager* undoManager = nullptr)
    {
        setBoolProp(state, SlotIDs::sipMuted(slotIdx), isMuted, undoManager);
    }

    static inline void setSlotCustomName(juce::ValueTree& state, int slotIdx, const juce::String& name, juce::UndoManager* undoManager = nullptr)
    {
		juce::String usableName = (name.length() > PluginConstants::maxSlotNameLength) 
            ? name.substring(0, PluginConstants::maxSlotNameLength) : name;

        setStringProp(state, SlotIDs::slotName(slotIdx), usableName, undoManager);
    }

    static inline void setSlotColour(juce::ValueTree& state, int slotIdx, const juce::String& hexColour, juce::UndoManager* undoManager = nullptr)
    {
        setStringProp(state, SlotIDs::slotColour(slotIdx), hexColour, undoManager);
    }

    static inline void setGroupColour(juce::ValueTree& state, int grpId, int colourIdx, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::groupColour(grpId), colourIdx, undoManager);
    }

    static inline void setVcaName(juce::ValueTree& state, int vcaIdx, const juce::String& name, juce::UndoManager* undoManager = nullptr)
    {
        juce::String usableName = (name.length() > PluginConstants::maxGroupNameLength)
            ? name.substring(0, PluginConstants::maxGroupNameLength) : name;

        setStringProp(state, SlotIDs::vcaName(vcaIdx), usableName, undoManager);
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

    static inline void setPluginConnected(juce::ValueTree& state, bool isConnected, juce::UndoManager* undoManager = nullptr)
    {
        setBoolProp(state, SlotIDs::isConnected().toString(), isConnected, undoManager);
    }

    static inline void setLinkPolarityInverse(juce::ValueTree& state, int slotIdx, bool isInverse, juce::UndoManager* undoManager = nullptr) {
        setBoolProp(state, SlotIDs::linkPolarityInverse(slotIdx), isInverse, undoManager);
    }

    static inline void setLinkMaskVolume(juce::ValueTree& state, int slotIdx, bool isLinked, juce::UndoManager* undoManager = nullptr) {
        setBoolProp(state, SlotIDs::linkMaskVolume(slotIdx), isLinked, undoManager);
    }

    static inline void setLinkMaskMute(juce::ValueTree& state, int slotIdx, bool isLinked, juce::UndoManager* undoManager = nullptr) {
        setBoolProp(state, SlotIDs::linkMaskMute(slotIdx), isLinked, undoManager);
    }

    static inline void setLinkMaskSolo(juce::ValueTree& state, int slotIdx, bool isLinked, juce::UndoManager* undoManager = nullptr) {
        setBoolProp(state, SlotIDs::linkMaskSolo(slotIdx), isLinked, undoManager);
    }

    static inline void setLinkMaskPan(juce::ValueTree& state, int slotIdx, bool isLinked, juce::UndoManager* undoManager = nullptr) {
        setBoolProp(state, SlotIDs::linkMaskPan(slotIdx), isLinked, undoManager);
    }

    static inline void setCustomLinkedId(juce::ValueTree& state, int slotIdx, int linkedIdx, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::customLinkedId(slotIdx), linkedIdx, undoManager);
    }

    static inline void setCustomLinkedIsVca(juce::ValueTree& state, int slotIdx, bool isVca, juce::UndoManager* undoManager = nullptr)
    {
        setBoolProp(state, SlotIDs::customLinkedIsVca(slotIdx), isVca, undoManager);
    }

    static inline void setLinkColourIndex(juce::ValueTree& state, int slotIdx, int colourIdx, juce::UndoManager* undoManager = nullptr)
    {
        setIntProp(state, SlotIDs::linkColourIndex(slotIdx), colourIdx, undoManager);
    }

    // =========================================================================
    // APVTS PARAMETER Getters (Audio Parameters)
    // =========================================================================

    static inline float getRawParamValue(const juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
    {
        if (auto* param = apvts.getRawParameterValue(paramId))
            return param->load();
        return 0.0f;
    }

    static inline float denormalizeValue(const juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, float normalizedValue)
    {
        if (auto* param = apvts.getParameter(paramId))
            return param->convertFrom0to1(normalizedValue);
        return 0.0f;
    }

    static inline int getActiveStoreId(const juce::AudioProcessorValueTreeState& apvts)
    {
        return juce::roundToInt(getRawParamValue(apvts, PresetTags::ActiveStoreParamId));
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

    static inline bool isSlotSoloSafe(const juce::AudioProcessorValueTreeState& apvts, int slotIdx)
    {
        return getRawParamValue(apvts, SlotIDs::soloSafe(slotIdx)) > 0.5f;
    }

    static inline bool isSlotSoloed(const juce::AudioProcessorValueTreeState& apvts, int slotIdx)
    {
        return getRawParamValue(apvts, SlotIDs::solo(slotIdx)) > 0.5f;
    }

    static inline bool isSlotMuted(const juce::AudioProcessorValueTreeState& apvts, int slotIdx)
    {
        return getRawParamValue(apvts, SlotIDs::mute(slotIdx)) > 0.5f;
    }

    // =========================================================================
    // APVTS PARAMETER Setters (Audio Parameters)
    // =========================================================================

    static inline void setParamNormalized(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, float normalizedValue)
    {
        if (auto* param = apvts.getParameter(paramId))
            param->setValueNotifyingHost(normalizedValue);
    }

    static inline void setParamNormalizedIfChanged(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, float normalizedValue)
    {
        if (auto* param = apvts.getParameter(paramId))
        {
            if (param->getValue() != normalizedValue)
                param->setValueNotifyingHost(normalizedValue);
        }
    }

    static inline void setParamUnnormalized(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, float unnormalizedValue)
    {
        if (auto* param = apvts.getParameter(paramId))
            param->setValueNotifyingHost(param->convertTo0to1(unnormalizedValue));
    }

    static inline void setActiveStoreId(juce::AudioProcessorValueTreeState& apvts, int index)
    {
        setParamUnnormalized(apvts, PresetTags::ActiveStoreParamId, static_cast<float>(index));
    }

    static inline void setSlotActive(juce::AudioProcessorValueTreeState& apvts, int slotIdx, bool shouldBeActive)
    {
        setParamNormalizedIfChanged(apvts, SlotIDs::isActive(slotIdx), shouldBeActive ? 1.0f : 0.0f);
    }

    static inline void setSlotSoloSafe(juce::AudioProcessorValueTreeState& apvts, int slotIdx, bool shouldBeSoloSafe)
    {
        setParamNormalizedIfChanged(apvts, SlotIDs::soloSafe(slotIdx), shouldBeSoloSafe ? 1.0f : 0.0f);
    }

    static inline void setSlotSoloed(juce::AudioProcessorValueTreeState& apvts, int slotIdx, bool shouldBeSoloed)
    {
        setParamNormalizedIfChanged(apvts, SlotIDs::solo(slotIdx), shouldBeSoloed ? 1.0f : 0.0f);
    }

    static inline void setSlotMuted(juce::AudioProcessorValueTreeState& apvts, int slotIdx, bool shouldBeMuted)
    {
        setParamNormalizedIfChanged(apvts, SlotIDs::mute(slotIdx), shouldBeMuted ? 1.0f : 0.0f);
    }

    static inline void initSliderFromParam(const juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, juce::Slider& slider)
    {
        if (auto* param = apvts.getParameter(paramId))
        {
            auto range = param->getNormalisableRange();
            slider.setRange(range.start, range.end, range.interval);
            slider.setValue(param->convertFrom0to1(param->getValue()), juce::dontSendNotification);
        }
    }

    // =========================================================================
    // EXTRAS
    // =========================================================================

    static inline int getIndexFromParamId(const juce::String& paramId, const juce::String& prefix)
    {
        return paramId.substring(prefix.length()).getIntValue();
    }

    static inline void unlinkStereoSlot(juce::ValueTree& state, int slotIdx, juce::UndoManager* undoManager = nullptr)
    {
        removeProp(state, SlotIDs::isStereoLinked(slotIdx), undoManager);
        removeProp(state, SlotIDs::isStereoMain(slotIdx), undoManager);
        removeProp(state, SlotIDs::linkedSlotId(slotIdx), undoManager);
    }

    static inline bool isStereoProperty(const juce::String& propName)
    {
        return propName.startsWith(SlotIdStringPrefixes::isStereo) ||
            propName.startsWith(SlotIdStringPrefixes::linkedSlotId) ||
            propName.startsWith(SlotIdStringPrefixes::xpStereo);
    }

    static inline bool isGroupProperty(const juce::String& propName)
    {
        return propName.startsWith(SlotIdStringPrefixes::group) ||
            propName.startsWith(SlotIdStringPrefixes::vcaName);
    }

    static inline bool isStereoOrGroupProperty(const juce::String& propName)
    {
        return isStereoProperty(propName) || isGroupProperty(propName);
    }

    namespace detail
    {
        static inline int findManualGroupColour(const juce::ValueTree& state, int targetGrpId, std::array<bool, GroupColours::numColours>& isColourUsed)
        {
            for (int i = 1; i <= PluginConstants::numGroups; ++i)
            {
                juce::Identifier propId = SlotIDs::groupColour(i);

                if (state.hasProperty(propId))
                {
                    int manualColourIndex = static_cast<int>(state.getProperty(propId));

                    if (manualColourIndex >= 0 && manualColourIndex < GroupColours::numColours)
                    {
                        isColourUsed[manualColourIndex] = true;

                        if (i == targetGrpId)
                            return manualColourIndex;
                    }
                }
            }
            return -1;
        }

        static inline int resolveAutoGroupColour(const juce::ValueTree& state, int targetGrpId, std::array<bool, GroupColours::numColours>& isColourUsed)
        {
            int numStandardColours = GroupColours::numColours - 1;
            int whiteIndex = GroupColours::numColours - 1;

            for (int i = 1; i <= targetGrpId; ++i)
            {
                juce::Identifier propId = SlotIDs::groupColour(i);

                if (!state.hasProperty(propId))
                {
                    int startIndex = (i - 1) % numStandardColours;
                    int assignedColourIndex = whiteIndex;

                    for (int c = 0; c < numStandardColours; ++c)
                    {
                        int testIndex = (startIndex + c) % numStandardColours;
                        if (!isColourUsed[testIndex])
                        {
                            assignedColourIndex = testIndex;
                            isColourUsed[testIndex] = true;
                            break;
                        }
                    }

                    if (i == targetGrpId)
                        return assignedColourIndex;
                }
            }
            return whiteIndex;
        }
    }

}