#pragma once
#include <JuceHeader.h>
#include <array>
#include "../Enums/SlotMode.h"

class GlobalSlotRegistry : public juce::ChangeBroadcaster
{
public:
    GlobalSlotRegistry();
    ~GlobalSlotRegistry() = default;

    void claimSlot(int slotIndex, const juce::Uuid& instanceId);
    void releaseSlot(int slotIndex, const juce::Uuid& instanceId);
    SlotMode getSlotMode(int slotIndex, const juce::Uuid& instanceId, bool isLocallyActive);

private:
    int getArrayIndex(int slotId) const { return slotId - 1; }
    bool isValidSlot(int slotId) const 
    {
        return juce::isPositiveAndBelow(getArrayIndex(slotId), slotOwners.size()); 
    }

    juce::CriticalSection lock;
    juce::Array<juce::Uuid> slotOwners;
};