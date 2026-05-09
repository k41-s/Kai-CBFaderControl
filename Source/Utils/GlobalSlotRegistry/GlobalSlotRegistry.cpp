#include "GlobalSlotRegistry.h"
#include "../../UI/Components/UIConstants.h"

GlobalSlotRegistry::GlobalSlotRegistry()
{
    slotOwners.insertMultiple(0, juce::Uuid::null(), PluginConstants::numSlots);
}

void GlobalSlotRegistry::claimSlot(int slotIndex, const juce::Uuid& instanceId)
{
    if (!isValidSlot(slotIndex))
        return;

    bool changed = false;
    {
        juce::ScopedLock sl(lock);
        if (slotOwners[getArrayIndex(slotIndex)] != instanceId)
        {
            slotOwners.set(getArrayIndex(slotIndex), instanceId);
            changed = true;
        }
    }

    if (changed)
        sendChangeMessage();
}

void GlobalSlotRegistry::releaseSlot(int slotIndex, const juce::Uuid& instanceId)
{
    if (!isValidSlot(slotIndex))
        return;

    juce::ScopedLock sl(lock);
    if (slotOwners[getArrayIndex(slotIndex)] == instanceId)
    {
        slotOwners.set(getArrayIndex(slotIndex), juce::Uuid::null());
        sendChangeMessage();
    }
}

SlotMode GlobalSlotRegistry::getSlotMode(int slotIndex, const juce::Uuid& instanceId, bool isLocallyActive)
{
    if (!isLocallyActive)
        return SlotMode::Disabled;

    if (!isValidSlot(slotIndex))
        return SlotMode::Disabled;

    juce::ScopedLock sl(lock);
    if (slotOwners[getArrayIndex(slotIndex)] == instanceId)
        return SlotMode::FullAccess;

    return SlotMode::ReadOnly;
}