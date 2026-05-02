#pragma once
#include <JuceHeader.h>
#include <array>

class GlobalSlotRegistry
{
public:
	GlobalSlotRegistry();
	bool claimSlot(int slotIndex, void* processorInstance);
	void releaseSlot(int slotIndex, void* processorInstance);
	bool isClaimedByOther(int slotIndex, void* processorInstance);
	void releaseAllForInstance(void* processorInstance);
private:
	std::array<void*, 32> owners;
	juce::CriticalSection lock;
};