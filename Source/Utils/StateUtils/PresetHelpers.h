#pragma once
#include <JuceHeader.h>
#include "../../Main/PresetManager/PresetManager.h"
#include "../../Main/SlotIDs.h"

namespace PresetHelpers
{
	// Extract all relevant strings to const vars, currently only slotOrders_

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

            for (int i = 0; i < incomingState.getNumProperties(); ++i)
            {
                juce::Identifier propName = incomingState.getPropertyName(i);
                juce::String propStr = propName.toString();

                bool isLayoutProp = propStr.startsWith(SlotIdStringPrefixes::slotName) ||
                    propStr.startsWith(SlotIdStringPrefixes::slotColour) ||
                    propStr.startsWith("slotOrder_");

                bool isDataProp = propStr.startsWith(SlotIdStringPrefixes::volume) ||
                    propStr.startsWith(SlotIdStringPrefixes::mute) ||
                    propStr.startsWith(SlotIdStringPrefixes::pan) ||
                    propStr.startsWith(SlotIdStringPrefixes::solo) ||
                    propStr.startsWith(SlotIdStringPrefixes::vcaVolume) ||
                    propStr.startsWith(SlotIdStringPrefixes::vcaMute);

                if ((loadLayout && isLayoutProp) || (loadData && isDataProp))
                {
                    currentState.setProperty(propName, incomingState.getProperty(propName), nullptr);
                }
            }

            apvts.replaceState(currentState);
        }

        if (loadStores)
        {
            auto* storesXml = rootXml.getChildByName(PresetTags::StoresTreeType.toString());
            if (storesXml != nullptr)
            {
                presetManager.loadFromXml(storesXml);
            }
        }
    }
}