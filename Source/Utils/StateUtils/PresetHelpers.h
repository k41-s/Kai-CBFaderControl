#pragma once
#include <JuceHeader.h>
#include "../../Main/PresetManager/PresetManager.h"
#include "../../Main/SlotIDs.h"

namespace PresetHelpers
{
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

            // =====================================================================
            // 1. LAYOUT RECALL: Loop over Properties (Names, Colours, Groups, etc.)
            // =====================================================================
            for (int i = 0; i < incomingState.getNumProperties(); ++i)
            {
                juce::Identifier propName = incomingState.getPropertyName(i);
                juce::String propStr = propName.toString();

                bool isLayoutProp = propStr.startsWith(SlotIdStringPrefixes::slotName) ||
                    propStr.startsWith(SlotIdStringPrefixes::slotColour) ||
                    propStr.startsWith("slotOrder") || // const variables for all these and everything possible
                    propStr.startsWith("group") ||
                    propStr.startsWith("isStereo") ||
                    propStr.startsWith("linkedSlot");

                if (loadLayout && isLayoutProp)
                {
                    currentState.setProperty(propName, incomingState.getProperty(propName), nullptr);
                }
            }

            // =====================================================================
            // 2. PARAM RECALL: Loop over Children (<PARAM id="volume_1" value="0.0"/>)
            // =====================================================================
            for (int i = 0; i < incomingState.getNumChildren(); ++i)
            {
                juce::ValueTree incomingChild = incomingState.getChild(i);
                if (incomingChild.hasType("PARAM"))
                {
                    juce::String paramId = incomingChild.getProperty("id").toString();

                    // 1. Define what counts as a DATA parameter (Audio logic)
                    bool isDataParam = paramId.startsWith(SlotIdStringPrefixes::volume) ||
                        paramId.startsWith(SlotIdStringPrefixes::mute) ||
                        paramId.startsWith(SlotIdStringPrefixes::pan) ||
                        paramId.startsWith(SlotIdStringPrefixes::solo) ||
                        paramId.startsWith(SlotIdStringPrefixes::vcaVolume) ||
                        paramId.startsWith(SlotIdStringPrefixes::vcaMute);

                    // 2. Define what counts as a LAYOUT parameter (UI / Surface logic)
                    // (Adjust "isActive" if your SlotIDs.h uses a different string prefix for active faders)
                    bool isLayoutParam = paramId.startsWith("isActive") ||
                        paramId.startsWith("vcaExpanded");

                    // 3. Define what counts as a STORE parameter
                    bool isStoreParam = paramId.startsWith("activeSnapshot") ||
                        paramId.startsWith("activeStore");

                    // Apply Data parameters
                    if (loadData && isDataParam)
                    {
                        auto currentChild = currentState.getChildWithProperty("id", paramId);
                        if (currentChild.isValid())
                            currentChild.setProperty("value", incomingChild.getProperty("value"), nullptr);
                    }

                    // Apply Layout parameters stored as APVTS floats/bools
                    if (loadLayout && isLayoutParam)
                    {
                        auto currentChild = currentState.getChildWithProperty("id", paramId);
                        if (currentChild.isValid())
                            currentChild.setProperty("value", incomingChild.getProperty("value"), nullptr);
                    }

                    // Apply active snapshot/store pointers
                    if (loadStores && isStoreParam)
                    {
                        auto currentChild = currentState.getChildWithProperty("id", paramId);
                        if (currentChild.isValid())
                            currentChild.setProperty("value", incomingChild.getProperty("value"), nullptr);
                    }
                }
            }

            // Merge everything back into the live APVTS
            apvts.replaceState(currentState);
        }

        // =====================================================================
        // 3. STORE RECALL: Extract Snapshot Stores
        // =====================================================================
        if (loadStores)
        {
            juce::XmlElement rootCopy(rootXml);
            presetManager.loadFromXml(&rootCopy);
        }
    }
}