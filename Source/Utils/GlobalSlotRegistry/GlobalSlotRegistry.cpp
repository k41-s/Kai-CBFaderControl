#include "GlobalSlotRegistry.h"

GlobalSlotRegistry::GlobalSlotRegistry() 
{ 
    owners.fill(nullptr);
}

bool GlobalSlotRegistry::claimSlot(int slotIndex, void* processorInstance)
{
    juce::ScopedLock sl(lock);
    int idx = slotIndex - 1;

    if (owners[idx] != nullptr && owners[idx] != processorInstance)
        return false;

    owners[idx] = processorInstance;
    return true;
}

void GlobalSlotRegistry::releaseSlot(int slotIndex, void* processorInstance)
{
    juce::ScopedLock sl(lock);
    int idx = slotIndex - 1;
    if (owners[idx] == processorInstance)
        owners[idx] = nullptr;
}

bool GlobalSlotRegistry::isClaimedByOther(int slotIndex, void* processorInstance)
{
    juce::ScopedLock sl(lock);
    return (owners[slotIndex - 1] != nullptr && owners[slotIndex - 1] != processorInstance);
}

void GlobalSlotRegistry::releaseAllForInstance(void* processorInstance)
{
    juce::ScopedLock sl(lock);
    for (int i = 0; i < 32; ++i) 
    {
        if (owners[i] == processorInstance)
            owners[i] = nullptr;
    }
}