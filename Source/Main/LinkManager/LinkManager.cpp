#include "LinkManager.h"
#include "../SlotIDs.h"
#include "../PluginProcessor/PluginProcessor.h"
#include "../../Utils/StateUtils/SlotStateHelpers.h"
#include "../../Utils/ScopedAtomicSetter.h"

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
		setUnclampedPan(i, initialPan);
    }
}

void LinkManager::addVcaMasterListeners()
{
    for (int i = 1; i <= PluginConstants::numVcas; ++i) 
    {
        processor.apvts.addParameterListener(SlotIDs::vcaVolume(i), this);
        processor.apvts.addParameterListener(SlotIDs::vcaMute(i), this);

        setLastVcaVolume(i, SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::vcaVolume(i)));
		setUnclampedVcaVolume(i, SlotStateHelpers::getRawParamValue(processor.apvts, SlotIDs::vcaVolume(i)));
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
		setUnclampedPan(slotIdx, rawValue);
    }
    else if (parameterID.startsWith(SlotIdStringPrefixes::vcaVolume)) 
    {
        int grpIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::vcaVolume);
        setLastVcaVolume(grpIdx, rawValue);
        setUnclampedVcaVolume(grpIdx, rawValue);
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
    setUnclampedPan(slotIdx, newValue);

    if (!isPropagatingCustomLink)
        propagateCustomLinkPan(slotIdx, false, delta);
}

void LinkManager::handleVcaVolumeParameterChanged(const juce::String& parameterID, float rawValue)
{
    int grpIdx = SlotStateHelpers::getIndexFromParamId(parameterID, SlotIdStringPrefixes::vcaVolume);
    float delta = rawValue - getLastVcaVolume(grpIdx);
    if (std::abs(delta) < 0.001f) return;

    setLastVcaVolume(grpIdx, rawValue);
    setUnclampedVcaVolume(grpIdx, rawValue);

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
    ScopedAtomicSetter setter(isPropagatingGroup, true);
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        int assignedGrp = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        if (assignedGrp == grpIdx)
        {
            applyGroupVolumeDeltaToSlot(i, delta);
        }
    }
}

void LinkManager::syncGroupMutesWithVca(int grpIdx, float newValue)
{
    ScopedAtomicSetter setter(isPropagatingGroup, true);
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        int assignedGrp = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        if (assignedGrp == grpIdx)
        {
            SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::mute(i), newValue);
        }
    }
}

void LinkManager::applyDeltaToGroupMembers(int slotIdx, int grpId, float delta)
{
    ScopedAtomicSetter setter(isPropagatingGroup, true);
    forEachGroupMember(slotIdx, grpId, [&](int memberIdx) 
    {
        applyGroupVolumeDeltaToSlot(memberIdx, delta);
    });
}

void LinkManager::syncMutesWithinGroup(int slotIdx, int grpId, float newValue)
{
    ScopedAtomicSetter setter(isPropagatingGroup, true);
    forEachGroupMember(slotIdx, grpId, [&](int memberIdx) 
    {
        SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::mute(memberIdx), newValue);
    });
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

    ScopedAtomicSetter setter(isPropagatingGroup, true);
    ScopedAtomicSetter linkSetter(isPropagatingCustomLink, true);

    applyOrReleaseSIPMutes(anySoloActive);
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
    applyVolumeDelta(slotIdx, false, delta);
}

void LinkManager::propagateCustomLinkVolume(int sourceTrueId, bool isSourceVca, float delta)
{
    auto route = getLinkRouteDetails(sourceTrueId, isSourceVca);

    if (route.targetId == 0 || !SlotStateHelpers::isLinkMaskVolume(processor.apvts.state, route.sourceTreeId))
        return;

    float appliedDelta = applyPolarity(delta, route.sourceTreeId);

    ScopedAtomicSetter setter(isPropagatingCustomLink, true);
    applyVolumeDelta(route.targetId, route.isTargetVca, appliedDelta);
}

void LinkManager::propagateCustomLinkMute(int sourceTrueId, bool isSourceVca, float newValue)
{
    auto route = getLinkRouteDetails(sourceTrueId, isSourceVca);

    if (route.targetId == 0 || !SlotStateHelpers::isLinkMaskMute(processor.apvts.state, route.sourceTreeId))
        return;

    ScopedAtomicSetter setter(isPropagatingCustomLink, true);

    if (route.isTargetVca)
        SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::vcaMute(route.targetId), newValue);
    else
        SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::mute(route.targetId), newValue);
}

void LinkManager::propagateCustomLinkSolo(int sourceTrueId, bool isSourceVca, float newValue)
{
    auto route = getLinkRouteDetails(sourceTrueId, isSourceVca);

    if (route.targetId == 0 || route.isTargetVca || !SlotStateHelpers::isLinkMaskSolo(processor.apvts.state, route.sourceTreeId))
        return;

    ScopedAtomicSetter setter(isPropagatingCustomLink, true);
    SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::solo(route.targetId), newValue);
}

void LinkManager::propagateCustomLinkPan(int sourceTrueId, bool isSourceVca, float delta)
{
    auto route = getLinkRouteDetails(sourceTrueId, isSourceVca);

    if (route.targetId == 0 || route.isTargetVca || !SlotStateHelpers::isLinkMaskPan(processor.apvts.state, route.sourceTreeId))
        return;

    float appliedDelta = applyPolarity(delta, route.sourceTreeId);

    ScopedAtomicSetter setter(isPropagatingCustomLink, true);
    applyPanDelta(route.targetId, appliedDelta);
}

LinkManager::LinkRouteDetails LinkManager::getLinkRouteDetails(int sourceTrueId, bool isSourceVca) const
{
    LinkRouteDetails route;
    route.sourceTreeId = isSourceVca 
        ? sourceTrueId + PluginConstants::vcaSelectionOffset 
        : sourceTrueId;
    route.targetId = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, route.sourceTreeId);

    if (route.targetId != 0) 
    {
        route.isTargetVca = SlotStateHelpers::getCustomLinkedIsVca(processor.apvts.state, route.sourceTreeId);
        
        if (!isSourceVca && !route.isTargetVca)
        {
            int sourceGrpId = SlotStateHelpers::getGroupId(processor.apvts.state, sourceTrueId);
            int targetGrpId = SlotStateHelpers::getGroupId(processor.apvts.state, route.targetId);

            if (sourceGrpId > 0 && sourceGrpId == targetGrpId)
            {
                route.targetId = 0;
            }
        }
    }
    return route;
}

void LinkManager::applyVolumeDelta(int targetId, bool isVca, float delta)
{
    float currentUnclamped = isVca ? getUnclampedVcaVolume(targetId) : getUnclampedVolume(targetId);
    float newUnclamped = currentUnclamped + delta;
    float clampedVol = juce::jlimit(PluginConstants::volumeMin, PluginConstants::volumeMax, newUnclamped);

    if (isVca)
    {
        setUnclampedVcaVolume(targetId, newUnclamped);
        SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::vcaVolume(targetId), clampedVol);
        setLastVcaVolume(targetId, clampedVol);
    }
    else
    {
        setUnclampedVolume(targetId, newUnclamped);
        SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::volume(targetId), clampedVol);
        setLastVolume(targetId, clampedVol);
    }
}

void LinkManager::applyPanDelta(int targetId, float delta)
{
    float newUnclamped = getUnclampedPan(targetId) + delta;
    setUnclampedPan(targetId, newUnclamped);
    float clampedPan = juce::jlimit(-1.0f, 1.0f, newUnclamped);

    SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::pan(targetId), clampedPan);
    setLastPan(targetId, clampedPan);
}

float LinkManager::applyPolarity(float delta, int sourceTreeId) const
{
    return SlotStateHelpers::isLinkPolarityInverse(processor.apvts.state, sourceTreeId) 
        ? -delta 
        : delta;
}

void LinkManager::forEachGroupMember(int leaderSlotIdx, int grpId, std::function<void(int)> action) const
{
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        if (i == leaderSlotIdx) continue;

        int otherGrpId = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        GroupRole otherRoleId = SlotStateHelpers::getGroupRole(processor.apvts.state, i);

        if (otherGrpId == grpId && otherRoleId == GroupRole::Member)
        {
            action(i);
        }
    }
}