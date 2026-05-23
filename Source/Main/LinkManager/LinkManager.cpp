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
        processor.apvts.addParameterListener(SlotIDs::solo(i), this);
        processor.apvts.addParameterListener(SlotIDs::soloSafe(i), this);

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
        processor.apvts.removeParameterListener(SlotIDs::solo(i), this);
        processor.apvts.removeParameterListener(SlotIDs::soloSafe(i), this);
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
    float rawValue = newValue;

    if (processor.isRestoringState || isPropagating)
    {
        handleProcessorRestoringState(parameterID, rawValue);
        return;
    }

    if (parameterID.startsWith(SlotIdStringPrefixes::vcaVolume)) 
        handleVcaVolumeParameterChanged(parameterID, rawValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::vcaMute))
        handleVcaMuteParameterChanged(parameterID, newValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::volume)) 
        handleVolumeParameterChanged(parameterID, rawValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::mute))
        handleMuteParameterChanged(parameterID, newValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::solo) ||
        parameterID.startsWith(SlotIdStringPrefixes::soloSafe))
    {
        updateSipState();
    }
}

void LinkManager::handleProcessorRestoringState(const juce::String& parameterID, float rawValue)
{
    if (parameterID.startsWith(SlotIdStringPrefixes::volume)) 
    {
        int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::volume);
		setLastVolume(slotIdx, rawValue);
		setUnclampedVolume(slotIdx, rawValue);
    }
    else if (parameterID.startsWith(SlotIdStringPrefixes::vcaVolume)) 
    {
        int grpIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::vcaVolume);
        setLastVcaVolume(grpIdx, rawValue);
    }
}

void LinkManager::handleVcaVolumeParameterChanged(const juce::String& parameterID, float rawValue)
{
    int grpIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::vcaVolume);
    float delta = rawValue - getLastVcaVolume(grpIdx);

    if (std::abs(delta) < 0.001f) return;

    setLastVcaVolume(grpIdx, rawValue);
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

void LinkManager::handleVolumeParameterChanged(const juce::String& parameterID, float rawValue)
{
    int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::volume);
    float delta = rawValue - getLastVolume(slotIdx);

    if (std::abs(delta) < 0.001f) return;

    setLastVolume(slotIdx, rawValue);
    setUnclampedVolume(slotIdx, rawValue);

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
        GroupRole otherRoleId = SlotStateHelpers::getGroupRole(processor.apvts.state, i);

        if (otherGrpId == grpId && otherRoleId == GroupRole::Member)
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
        GroupRole otherRoleId = SlotStateHelpers::getGroupRole(processor.apvts.state, i);

        if (otherGrpId == grpId && otherRoleId == GroupRole::Member)
        {
            SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::mute(i), newValue);
        }
    }
	isPropagating = false;
}

void LinkManager::updateSipState()
{
    // Step A: Check if ANY slot in the entire plugin is soloed
    bool anySoloActive = false;
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        if (SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::solo(i)) > 0.5f)
        {
            anySoloActive = true;
            break;
        }
    }

    // Lock propagation so we don't trigger recursive group mute loops
    isPropagating = true;

    // Step B: Apply or Release SIP Mutes
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        bool isSoloed = SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::solo(i)) > 0.5f;
        bool isSafe = SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::soloSafe(i)) > 0.5f;
        bool isCurrentlyMuted = SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::mute(i)) > 0.5f;
        bool isSipMuted = SlotStateHelpers::isSipMuted(processor.apvts.state, i);

        if (anySoloActive)
        {
            if (!isSoloed && !isSafe)
            {
                handleNotCurrentlyMuted(isCurrentlyMuted, i);
            }
            else
            {
				handleIsSipMuted(isSipMuted, i);
            }
        }
        else
        {
            handleIsSipMuted(isSipMuted, i);
        }
    }

    isPropagating = false;
}

void LinkManager::handleNotCurrentlyMuted(bool isCurrentlyMuted, int i)
{
    if (!isCurrentlyMuted)
    {
        handleSIP(i, true);
    }
}

void LinkManager::handleIsSipMuted(bool isSipMuted, int i)
{
    if (isSipMuted)
    {
        handleSIP(i, false);
    }
}

void LinkManager::handleSIP(int i, bool mute)
{
    SlotStateHelpers::setSipMuted(processor.apvts.state, i, mute);
    SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::mute(i), mute ? 1.0f : 0.0f);
}

void LinkManager::applyVolumeDeltaToSlot(int slotIdx, float delta)
{
    float newUnclamped = getUnclampedVolume(slotIdx) + delta;
    setUnclampedVolume(slotIdx, newUnclamped);

    float targetVol = juce::jlimit(PluginConstants::volumeMin, PluginConstants::volumeMax, newUnclamped);
    SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::volume(slotIdx), targetVol);
    setLastVolume(slotIdx, targetVol);
}
