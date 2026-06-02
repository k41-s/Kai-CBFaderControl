#pragma once
#include <JuceHeader.h>
#include "../../Main/PresetManager/PresetManager.h"
#include "../../Main/SlotIDs.h"

class PresetHelpers
{
public:
    PresetHelpers() = delete;

    static inline void selectivelyApplyState(juce::AudioProcessorValueTreeState& apvts,
        PresetManager& presetManager,
        const juce::XmlElement& rootXml,
        bool loadLayout,
        bool loadData,
        bool loadStores)
    {
        auto* apvtsXml = rootXml.getChildByName(apvts.state.getType());

        if (apvtsXml != nullptr)
        {
            juce::ValueTree incomingState = juce::ValueTree::fromXml(*apvtsXml);
            juce::ValueTree currentState = apvts.copyState();

            applyLayoutProperties(incomingState, loadLayout, currentState);
            applyParameters(incomingState, loadData, currentState, loadLayout, loadStores);
            apvts.replaceState(currentState);
        }
        applyStores(loadStores, rootXml, presetManager);
    }


private:

    static void applyLayoutProperties(juce::ValueTree& incomingState, bool loadLayout, juce::ValueTree& currentState)
    {
        if (!loadLayout) return;

        for (int i = 0; i < incomingState.getNumProperties(); ++i)
        {
            juce::Identifier propName = incomingState.getPropertyName(i);

            if (isLayoutProperty(propName.toString()))
            {
                currentState.setProperty(propName, incomingState.getProperty(propName), nullptr);
            }
        }
    }

    static void applyParameters(juce::ValueTree& incomingState, bool loadData, juce::ValueTree& currentState, bool loadLayout, bool loadStores)
    {
        for (int i = 0; i < incomingState.getNumChildren(); ++i)
        {
            juce::ValueTree incomingChild = incomingState.getChild(i);

            if (incomingChild.hasType(ApvtsXmlTags::Param))
            {
                juce::String paramId = incomingChild.getProperty(ApvtsXmlTags::Id).toString();

                bool applyData = loadData && isDataParam(paramId);
                bool applyLayout = loadLayout && isLayoutParam(paramId);
                bool applyStore = loadStores && isStoreParam(paramId);

                if (applyData || applyLayout || applyStore)
                {
                    auto currentChild = currentState.getChildWithProperty(ApvtsXmlTags::Id, paramId);
                    if (currentChild.isValid())
                    {
                        currentChild.setProperty(ApvtsXmlTags::Value, incomingChild.getProperty(ApvtsXmlTags::Value), nullptr);
                    }
                }
            }
        }
    }

    static void applyStores(bool loadStores, const juce::XmlElement& rootXml, PresetManager& presetManager)
    {
        if (loadStores)
        {
            juce::XmlElement rootCopy(rootXml);
            presetManager.loadFromXml(&rootCopy);
        }
    }

    // =====================================================================
    // CATEGORIZATION CHECKERS
    // =====================================================================

	static bool isLayoutProperty(const juce::String& propStr)
	{
		return propStr.startsWith(SlotIdStringPrefixes::slotName) ||
            propStr.startsWith(SlotIdStringPrefixes::slotColour) ||
            propStr.startsWith(SlotIdStringPrefixes::slotOrder) ||
            propStr.startsWith(SlotIdStringPrefixes::group) ||
            propStr.startsWith(SlotIdStringPrefixes::isStereo) ||
            propStr.startsWith(SlotIdStringPrefixes::linkedSlotId);
	}

    static bool isDataParam(const juce::String& paramId)
    {
        return paramId.startsWith(SlotIdStringPrefixes::volume) ||
            paramId.startsWith(SlotIdStringPrefixes::mute) ||
            paramId.startsWith(SlotIdStringPrefixes::pan) ||
            paramId.startsWith(SlotIdStringPrefixes::solo) ||
            paramId.startsWith(SlotIdStringPrefixes::vcaVolume) ||
            paramId.startsWith(SlotIdStringPrefixes::vcaMute);
    }

    static bool isLayoutParam(const juce::String& paramId)
    {
        return paramId.startsWith(SlotIdStringPrefixes::isActive) ||
            paramId.startsWith(SlotIdStringPrefixes::isVcaExpanded);
    }

    static bool isStoreParam(const juce::String& paramId)
    {
        return paramId.startsWith(PresetTags::ActiveSnapshotParamId) ||
            paramId.startsWith(PresetTags::ActiveStoreParamId);
    }
};