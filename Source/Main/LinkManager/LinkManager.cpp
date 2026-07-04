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
        processor.apvts.addParameterListener(SlotIDs::pan(i), this);

        float initialVol = SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::volume(i));
        setLastVolume(i, initialVol);
        setUnclampedVolume(i, initialVol);

        float initialPan = SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::pan(i));
        setLastPan(i, initialPan);
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
        processor.apvts.removeParameterListener(SlotIDs::pan(i), this);
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

    if (processor.isRestoringState)
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

    else if (parameterID.startsWith(SlotIdStringPrefixes::pan))
        handlePanParameterChanged(parameterID, newValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::solo))
        handleSoloParameterChanged(parameterID, newValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::soloSafe))
        updateSipState();
}

void LinkManager::handleProcessorRestoringState(const juce::String& parameterID, float rawValue)
{
    if (parameterID.startsWith(SlotIdStringPrefixes::volume)) 
    {
        int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::volume);
		setLastVolume(slotIdx, rawValue);
		setUnclampedVolume(slotIdx, rawValue);
    }
    else if (parameterID.startsWith(SlotIdStringPrefixes::pan))
    {
        int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::pan);
        setLastPan(slotIdx, rawValue);
    }
    else if (parameterID.startsWith(SlotIdStringPrefixes::vcaVolume)) 
    {
        int grpIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::vcaVolume);
        setLastVcaVolume(grpIdx, rawValue);
    }
}

void LinkManager::handleVolumeParameterChanged(const juce::String& parameterID, float rawValue)
{
    int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::volume);
    float delta = rawValue - getLastVolume(slotIdx);
    if (std::abs(delta) < 0.001f) return;
    
    setLastVolume(slotIdx, rawValue);
    setUnclampedVolume(slotIdx, rawValue);

    if (!isPropagatingCustomLink)
        propagateCustomLinkVolume(slotIdx, false, delta);

    if (!isPropagatingGroup)
    {
        if (SlotStateHelpers::isGroupLeader(processor.apvts.state, slotIdx))
        {
            int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
            applyDeltaToGroupMembers(slotIdx, grpId, delta);
        }
    }
}

void LinkManager::handleMuteParameterChanged(const juce::String& parameterID, float newValue)
{
    int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::mute);

    if (!isPropagatingCustomLink)
        propagateCustomLinkMute(slotIdx, false, newValue);

    if (!isPropagatingGroup)
    {
        if (SlotStateHelpers::isGroupLeader(processor.apvts.state, slotIdx))
        {
            int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
            syncMutesWithinGroup(slotIdx, grpId, newValue);
        }
    }
}

void LinkManager::handleSoloParameterChanged(const juce::String& parameterID, float newValue)
{
    int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::solo);

    if (!isPropagatingCustomLink)
        propagateCustomLinkSolo(slotIdx, false, newValue);

    if (!isPropagatingGroup)
        updateSipState();
}

void LinkManager::handlePanParameterChanged(const juce::String& parameterID, float newValue)
{
    int slotIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::pan);

    float delta = newValue - getLastPan(slotIdx);
    if (std::abs(delta) < 0.001f) return;

    setLastPan(slotIdx, newValue);

    if (!isPropagatingCustomLink)
        propagateCustomLinkPan(slotIdx, false, delta);
}

void LinkManager::handleVcaVolumeParameterChanged(const juce::String& parameterID, float rawValue)
{
    int grpIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::vcaVolume);
    float delta = rawValue - getLastVcaVolume(grpIdx);
    if (std::abs(delta) < 0.001f) return;

    setLastVcaVolume(grpIdx, rawValue);

    if (!isPropagatingCustomLink)
        propagateCustomLinkVolume(grpIdx, true, delta);

    if (!isPropagatingGroup)
        applyDeltaToGroupFromVca(grpIdx, delta);
}

void LinkManager::handleVcaMuteParameterChanged(const juce::String& parameterID, float newValue)
{
    int grpIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::vcaMute);

    if (!isPropagatingCustomLink)
        propagateCustomLinkMute(grpIdx, true, newValue);

    if (!isPropagatingGroup)
        syncGroupMutesWithVca(grpIdx, newValue);
}

void LinkManager::applyDeltaToGroupFromVca(int grpIdx, float delta)
{
    isPropagatingGroup = true;
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        int assignedGrp = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        if (assignedGrp == grpIdx)
        {
            applyGroupVolumeDeltaToSlot(i, delta);
        }
    }
    isPropagatingGroup = false;
}

void LinkManager::syncGroupMutesWithVca(int grpIdx, float newValue)
{
    isPropagatingGroup = true;
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        int assignedGrp = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        if (assignedGrp == grpIdx)
        {
            SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::mute(i), newValue);
        }
    }
    isPropagatingGroup = false;
}

void LinkManager::applyDeltaToGroupMembers(int slotIdx, int grpId, float delta)
{
	isPropagatingGroup = true;
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        if (i == slotIdx) continue;

        int otherGrpId = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        GroupRole otherRoleId = SlotStateHelpers::getGroupRole(processor.apvts.state, i);

        if (otherGrpId == grpId && otherRoleId == GroupRole::Member)
        {
            applyGroupVolumeDeltaToSlot(i, delta);
        }
    }
	isPropagatingGroup = false;
}

void LinkManager::syncMutesWithinGroup(int slotIdx, int grpId, float newValue)
{
	isPropagatingGroup = true;
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
	isPropagatingGroup = false;
}

void LinkManager::updateSipState()
{
    bool anySoloActive = false;
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        if (SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::solo(i)) > 0.5f)
        {
            anySoloActive = true;
            break;
        }
    }

    isPropagatingGroup = true;
    applyOrReleaseSIPMutes(anySoloActive);
    isPropagatingGroup = false;
}

void LinkManager::applyOrReleaseSIPMutes(bool anySoloActive)
{
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

void LinkManager::applyGroupVolumeDeltaToSlot(int slotIdx, float delta)
{
    float newUnclamped = getUnclampedVolume(slotIdx) + delta;
    setUnclampedVolume(slotIdx, newUnclamped);
    float targetVol = juce::jlimit(PluginConstants::volumeMin, PluginConstants::volumeMax, newUnclamped);

    SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::volume(slotIdx), targetVol);
    setLastVolume(slotIdx, targetVol);
}

void LinkManager::propagateCustomLinkVolume(int sourceTrueId, bool isSourceVca, float delta)
{
    int sourceTreeId = isSourceVca 
        ? sourceTrueId + PluginConstants::vcaSelectionOffset 
        : sourceTrueId;

    int targetId = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, sourceTreeId);

    if (targetId == 0 || !SlotStateHelpers::isLinkMaskVolume(processor.apvts.state, sourceTreeId))
        return;

    float appliedDelta = SlotStateHelpers::isLinkPolarityInverse(processor.apvts.state, sourceTreeId) 
        ? (-1.0f * delta) 
        
        : delta;
    bool isTargetVca = SlotStateHelpers::getCustomLinkedIsVca(processor.apvts.state, sourceTreeId);

    isPropagatingCustomLink = true;

    if (isTargetVca)
    {
        float currentVol = getLastVcaVolume(targetId);
        float targetVol = juce::jlimit(PluginConstants::volumeMin, PluginConstants::volumeMax, currentVol + appliedDelta);
        SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::vcaVolume(targetId), targetVol);
        setLastVcaVolume(targetId, targetVol);
    }
    else
    {
        float newUnclamped = getUnclampedVolume(targetId) + appliedDelta;
        setUnclampedVolume(targetId, newUnclamped);
        float targetVol = juce::jlimit(PluginConstants::volumeMin, PluginConstants::volumeMax, newUnclamped);
        SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::volume(targetId), targetVol);
        setLastVolume(targetId, targetVol);
    }

    isPropagatingCustomLink = false;
}

void LinkManager::propagateCustomLinkMute(int sourceTrueId, bool isSourceVca, float newValue)
{
    int sourceTreeId = isSourceVca 
        ? sourceTrueId + PluginConstants::vcaSelectionOffset 
        : sourceTrueId;

    int targetId = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, sourceTreeId);

    if (targetId == 0 || !SlotStateHelpers::isLinkMaskMute(processor.apvts.state, sourceTreeId))
        return;

    bool isTargetVca = SlotStateHelpers::getCustomLinkedIsVca(processor.apvts.state, sourceTreeId);

    isPropagatingCustomLink = true;

    if (isTargetVca)
        SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::vcaMute(targetId), newValue);
    else
        SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::mute(targetId), newValue);

    isPropagatingCustomLink = false;
}

void LinkManager::propagateCustomLinkSolo(int sourceTrueId, bool isSourceVca, float newValue)
{
    int sourceTreeId = isSourceVca 
        ? sourceTrueId + PluginConstants::vcaSelectionOffset 
        : sourceTrueId;

    int targetId = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, sourceTreeId);

    if (targetId == 0 || !SlotStateHelpers::isLinkMaskSolo(processor.apvts.state, sourceTreeId))
        return;

    bool isTargetVca = SlotStateHelpers::getCustomLinkedIsVca(processor.apvts.state, sourceTreeId);

    if (!isTargetVca)
    {
        isPropagatingCustomLink = true;
        SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::solo(targetId), newValue);
        isPropagatingCustomLink = false;
    }
}

void LinkManager::propagateCustomLinkPan(int sourceTrueId, bool isSourceVca, float delta)
{
    int sourceTreeId = isSourceVca
        ? sourceTrueId + PluginConstants::vcaSelectionOffset
        : sourceTrueId;

    int targetId = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, sourceTreeId);
    if (targetId == 0 || !SlotStateHelpers::isLinkMaskPan(processor.apvts.state, sourceTreeId))
        return;

    bool isTargetVca = SlotStateHelpers::getCustomLinkedIsVca(processor.apvts.state, sourceTreeId);

    if (!isTargetVca)
    {
        isPropagatingCustomLink = true;

        float appliedDelta = SlotStateHelpers::isLinkPolarityInverse(processor.apvts.state, sourceTreeId)
            ? -delta
            : delta;

        float currentTargetPan = getLastPan(targetId);
        float newPan = juce::jlimit(-1.0f, 1.0f, currentTargetPan + appliedDelta);

        SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::pan(targetId), newPan);
        setLastPan(targetId, newPan);

        isPropagatingCustomLink = false;
    }
}