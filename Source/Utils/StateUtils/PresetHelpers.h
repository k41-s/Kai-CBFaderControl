#pragma once
#include <JuceHeader.h>
#include "../../Main/PresetManager/PresetManager.h"
#include "../../Main/SlotIDs.h"

struct RecallScope
{
    bool volume = true;
    bool mute = true;
    bool pan = true;
    bool solo = true;
    bool vca = true;
    bool name = true;
    bool colour = true;
    bool routing = true;
    bool activeState = true;
    bool stores = true;
};

class PresetHelpers
{
public:
    PresetHelpers() = delete;

    static inline void selectivelyApplyState(juce::AudioProcessorValueTreeState& apvts,
        PresetManager& presetManager,
        const juce::XmlElement& rootXml,
        const RecallScope& scope)
    {
        auto* apvtsXml = rootXml.getChildByName(apvts.state.getType());

        if (apvtsXml != nullptr)
        {
            juce::ValueTree incomingState = juce::ValueTree::fromXml(*apvtsXml);
            juce::ValueTree currentState = apvts.copyState();

            applyLayoutProperties(incomingState, scope, currentState);
            applyParameters(incomingState, scope, currentState);
            apvts.replaceState(currentState);
        }
        applyStores(scope.stores, rootXml, presetManager);
    }

private:
    static void applyLayoutProperties(juce::ValueTree& incomingState, const RecallScope& scope, juce::ValueTree& currentState)
    {
        for (int i = 0; i < incomingState.getNumProperties(); ++i)
        {
            juce::Identifier propName = incomingState.getPropertyName(i);
            juce::String propStr = propName.toString();
            bool apply = false;

            if (scope.name && propStr.startsWith(SlotIdStringPrefixes::slotName)) 
                apply = true;

            else if (scope.colour && propStr.startsWith(SlotIdStringPrefixes::slotColour)) 
                apply = true;

            else if (scope.routing && (propStr.startsWith(SlotIdStringPrefixes::group) ||
                propStr.startsWith(SlotIdStringPrefixes::isStereo) ||
                propStr.startsWith(SlotIdStringPrefixes::linkedSlotId) ||
                propStr.startsWith(SlotIdStringPrefixes::customLinkedId) ||
                propStr.startsWith(SlotIdStringPrefixes::customLinkedIsVca) ||
                propStr.startsWith(SlotIdStringPrefixes::slotOrder)))
            {
                apply = true;
            }

            if (apply)
            {
                currentState.setProperty(propName, incomingState.getProperty(propName), nullptr);
            }
        }
    }

    static void applyParameters(juce::ValueTree& incomingState, const RecallScope& scope, juce::ValueTree& currentState)
    {
        for (int i = 0; i < incomingState.getNumChildren(); ++i)
        {
            juce::ValueTree incomingChild = incomingState.getChild(i);

            if (incomingChild.hasType(ApvtsXmlTags::Param))
            {
                juce::String paramId = incomingChild.getProperty(ApvtsXmlTags::Id).toString();
                bool apply = false;

                if (scope.volume && paramId.startsWith(SlotIdStringPrefixes::volume)) 
                    apply = true;

                else if (scope.mute && paramId.startsWith(SlotIdStringPrefixes::mute)) 
                    apply = true;

                else if (scope.pan && paramId.startsWith(SlotIdStringPrefixes::pan)) 
                    apply = true;

                else if (scope.solo && paramId.startsWith(SlotIdStringPrefixes::solo))
                    apply = true;

                else if (scope.vca && (paramId.startsWith(SlotIdStringPrefixes::vcaVolume) ||
                    paramId.startsWith(SlotIdStringPrefixes::vcaMute))) 
                {
                    apply = true;
                }
                else if (scope.activeState && (paramId.startsWith(SlotIdStringPrefixes::isActive) ||
                    paramId.startsWith(SlotIdStringPrefixes::isVcaExpanded)))
                {
                    apply = true;
                }
                else if (scope.stores && (paramId.startsWith(PresetTags::ActiveSnapshotParamId) ||
                    paramId.startsWith(PresetTags::ActiveStoreParamId)))
                {
                    apply = true;
                }

                if (apply)
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
            auto* storesXml = rootXml.getChildByName(PresetTags::StoresTreeType.toString());

            if (storesXml != nullptr)
            {
                presetManager.loadFromXml(storesXml);
            }
        }
    }
};