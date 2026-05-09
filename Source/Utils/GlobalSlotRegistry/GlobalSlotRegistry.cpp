#include "GlobalSlotRegistry.h"
#include "../../UI/Components/UIConstants.h"

GlobalSlotRegistry::GlobalSlotRegistry()
{
    slotOwners.insertMultiple(0, juce::Uuid::null(), PluginConstants::numSlots);
}

void GlobalSlotRegistry::claimSlot(int slotIndex, const juce::Uuid& instanceId)
{
    if (!juce::isPositiveAndBelow(slotIndex - 1, slotOwners.size()))
        return;

    bool changed = false;
    {
        juce::ScopedLock sl(lock);
        if (slotOwners[slotIndex - 1] != instanceId)
        {
            slotOwners.set(slotIndex - 1, instanceId);
            changed = true;
        }
    }

    if (changed)
        sendChangeMessage();
}

void GlobalSlotRegistry::releaseSlot(int slotIndex, const juce::Uuid& instanceId)
{
    if (!juce::isPositiveAndBelow(slotIndex - 1, slotOwners.size()))
        return;

    juce::ScopedLock sl(lock);
    if (slotOwners[slotIndex - 1] == instanceId)
    {
        slotOwners.set(slotIndex - 1, juce::Uuid::null());
        sendChangeMessage();
    }
}

SlotMode GlobalSlotRegistry::getSlotMode(int slotIndex, const juce::Uuid& instanceId, bool isLocallyActive)
{
    if (!isLocallyActive)
        return SlotMode::Disabled;

    if (!juce::isPositiveAndBelow(slotIndex - 1, slotOwners.size()))
        return SlotMode::Disabled;

    juce::ScopedLock sl(lock);
    if (slotOwners[slotIndex - 1] == instanceId)
        return SlotMode::FullAccess;

    return SlotMode::ReadOnly;
}