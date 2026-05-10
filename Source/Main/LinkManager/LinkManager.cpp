#include "LinkManager.h"
#include "../SlotIDs.h"
#include "../PluginProcessor/PluginProcessor.h"
#include "../../Utils/StateUtils/SlotStateHelpers.h"

LinkManager::LinkManager(KaiCBFaderControlAudioProcessor& p) : processor(p)
{
    init();
}

void LinkManager::init()
{
    addRegularSlotListeners();
    addVcaMasterListeners();
}

void LinkManager::addRegularSlotListeners()
{
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        processor.apvts.addParameterListener(SlotIDs::volume(i), this);
        processor.apvts.addParameterListener(SlotIDs::mute(i), this);

        float initialVol = SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::volume(i));
        setLastVolume(i, initialVol);
        setUnclampedVolume(i, initialVol);
    }
}

void LinkManager::addVcaMasterListeners()
{
    for (int i = 1; i <= PluginConstants::numVcas; ++i) 
    {
        processor.apvts.addParameterListener(SlotIDs::vcaVolume(i), this);
        processor.apvts.addParameterListener(SlotIDs::vcaMute(i), this);

        setLastVcaVolume(i, SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::vcaVolume(i)));
    }
}

LinkManager::~LinkManager()
{
    removeRegularSlotListeners();
    removeVcaMasterListeners();
}

void LinkManager::removeRegularSlotListeners()
{
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        processor.apvts.removeParameterListener(SlotIDs::volume(i), this);
        processor.apvts.removeParameterListener(SlotIDs::mute(i), this);
    }
}

void LinkManager::removeVcaMasterListeners()
{
    for (int i = 1; i <= PluginConstants::numVcas; ++i) 
    {
        processor.apvts.removeParameterListener(SlotIDs::vcaVolume(i), this);
        processor.apvts.removeParameterListener(SlotIDs::vcaMute(i), this);
    }
}

void LinkManager::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (processor.isRestoringState || isPropagating)
    {
        handleProcessorRestoringState(parameterID, newValue);
        return;
    }

    if (parameterID.startsWith(SlotIdStringPrefixes::vcaVolume)) 
        handleVcaVolumeParameterChanged(parameterID, newValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::vcaMute))
        handleVcaMuteParameterChanged(parameterID, newValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::volume)) 
        handleVolumeParameterChanged(parameterID, newValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::mute))
        handleMuteParameterChanged(parameterID, newValue);
}

void LinkManager::handleProcessorRestoringState(const juce::String& parameterID, float newValue)
{
    if (parameterID.startsWith(SlotIdStringPrefixes::volume)) 
    {
        int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::volume);
		setLastVolume(slotIdx, newValue);
		setUnclampedVolume(slotIdx, newValue);
    }
    else if (parameterID.startsWith(SlotIdStringPrefixes::vcaVolume)) 
    {
        int grpIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::vcaVolume);
        setLastVcaVolume(grpIdx, newValue);
    }
}

void LinkManager::handleVcaVolumeParameterChanged(const juce::String& parameterID, float newValue)
{
    int grpIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::vcaVolume);
    float delta = newValue - getLastVcaVolume(grpIdx);
    setLastVcaVolume(grpIdx, newValue);

    applyDeltaToGroupFromVca(grpIdx, delta);
}

void LinkManager::applyDeltaToGroupFromVca(int grpIdx, float delta)
{
	isPropagating = true;
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        int assignedGrp = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        if (assignedGrp == grpIdx) 
        {
			applyVolumeDeltaToSlot(i, delta);
        }
    }
	isPropagating = false;
}

void LinkManager::handleVcaMuteParameterChanged(const juce::String& parameterID, float newValue)
{
    int grpIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::vcaMute);
    syncGroupMutesWithVca(grpIdx, newValue);
}

void LinkManager::syncGroupMutesWithVca(int grpIdx, float newValue)
{
	isPropagating = true;
    for (int i = 1; i <= PluginConstants::numSlots; ++i) 
    {
        int assignedGrp = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        if (assignedGrp == grpIdx) 
        {
            SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::mute(i), newValue);
        }
    }
	isPropagating = false;
}

void LinkManager::handleVolumeParameterChanged(const juce::String& parameterID, float newValue)
{
    int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::volume);

    float delta = newValue - getLastVolume(slotIdx);
    setLastVolume(slotIdx, newValue);
    setUnclampedVolume(slotIdx, newValue);

    if (SlotStateHelpers::isGroupLeader(processor.apvts.state, slotIdx))
    {
        int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
        applyDeltaToGroupMembers(slotIdx, grpId, delta);
    }
}

void LinkManager::applyDeltaToGroupMembers(int slotIdx, int grpId, float delta)
{
	isPropagating = true;
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        if (i == slotIdx) continue;

        int otherGrpId = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        int otherRoleId = SlotStateHelpers::getGroupRole(processor.apvts.state, i);

        if (otherGrpId == grpId && otherRoleId == 0)
        {
			applyVolumeDeltaToSlot(i, delta);
        }
    }
	isPropagating = false;
}

void LinkManager::handleMuteParameterChanged(const juce::String& parameterID, float newValue)
{
    int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::mute);

    if (SlotStateHelpers::isGroupLeader(processor.apvts.state, slotIdx)) 
    {
        int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
        syncMutesWithinGroup(slotIdx, grpId, newValue);
    }
}

void LinkManager::syncMutesWithinGroup(int slotIdx, int grpId, float newValue)
{
	isPropagating = true;
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        if (i == slotIdx) continue;

        int otherGrpId = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        int otherRoleId = SlotStateHelpers::getGroupRole(processor.apvts.state, i);

        if (otherGrpId == grpId && otherRoleId == 0) 
        {
            SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::mute(i), newValue);
        }
    }
	isPropagating = false;
}

void LinkManager::applyVolumeDeltaToSlot(int slotIdx, float delta)
{
    float newUnclamped = getUnclampedVolume(slotIdx) + delta;
    setUnclampedVolume(slotIdx, newUnclamped);

    float targetVol = juce::jlimit(-96.0f, 22.0f, newUnclamped);
    SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::volume(slotIdx), targetVol);
    setLastVolume(slotIdx, targetVol);
}
