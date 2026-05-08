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
    for (int i = 1; i <= 32; ++i) {
        processor.apvts.addParameterListener(SlotIDs::volume(i), this);
        processor.apvts.addParameterListener(SlotIDs::mute(i), this);

        lastVolume[i - 1] = processor.apvts.getRawParameterValue(SlotIDs::volume(i))->load();
    }
}

void LinkManager::addVcaMasterListeners()
{
    for (int i = 1; i <= 8; ++i) {
        processor.apvts.addParameterListener(SlotIDs::vcaVolume(i), this);
        processor.apvts.addParameterListener(SlotIDs::vcaMute(i), this);
        lastVcaVolume[i - 1] = processor.apvts.getRawParameterValue(SlotIDs::vcaVolume(i))->load();
    }
}

LinkManager::~LinkManager()
{
    removeRegularSlotListeners();
    removeVcaMasterListeners();
}

void LinkManager::removeRegularSlotListeners()
{
    for (int i = 1; i <= 32; ++i)
    {
        processor.apvts.removeParameterListener(SlotIDs::volume(i), this);
        processor.apvts.removeParameterListener(SlotIDs::mute(i), this);
    }
}

void LinkManager::removeVcaMasterListeners()
{
    for (int i = 1; i <= 8; ++i) {
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
    if (parameterID.startsWith(SlotIdStringPrefixes::volume)) {
        int slotIdx = parameterID.substring(7).getIntValue();
        lastVolume[slotIdx - 1] = newValue;
    }
    else if (parameterID.startsWith(SlotIdStringPrefixes::vcaVolume)) {
        int grpIdx = parameterID.substring(10).getIntValue();
        lastVcaVolume[grpIdx - 1] = newValue;
    }
}

void LinkManager::handleVcaVolumeParameterChanged(const juce::String& parameterID, float newValue)
{
    int grpIdx = parameterID.substring(10).getIntValue();
    float delta = newValue - lastVcaVolume[grpIdx - 1];
    lastVcaVolume[grpIdx - 1] = newValue;

    applyDeltaToGroupFromVca(grpIdx, delta);
}

void LinkManager::applyDeltaToGroupFromVca(int grpIdx, float delta)
{
	isPropagating = true;
    for (int i = 1; i <= 32; ++i)
    {
        int assignedGrp = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        if (assignedGrp == grpIdx) 
        {
            float targetVol = lastVolume[i - 1] + delta;
            targetVol = juce::jlimit(-96.0f, 22.0f, targetVol);

            SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::volume(i), targetVol);

            lastVolume[i - 1] = targetVol;
        }
    }
	isPropagating = false;
}

void LinkManager::handleVcaMuteParameterChanged(const juce::String& parameterID, float newValue)
{
    int grpIdx = parameterID.substring(8).getIntValue();
    syncGroupMutesWithVca(grpIdx, newValue);
}

void LinkManager::syncGroupMutesWithVca(int grpIdx, float newValue)
{
	isPropagating = true;
    for (int i = 1; i <= 32; ++i) {
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
    int slotIdx = parameterID.substring(7).getIntValue();

    float delta = newValue - lastVolume[slotIdx - 1];
    lastVolume[slotIdx - 1] = newValue;

    if (SlotStateHelpers::isGroupLeader(processor.apvts.state, slotIdx))
    {
        int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
        applyDeltaToGroupMembers(slotIdx, grpId, delta);
    }
}

void LinkManager::applyDeltaToGroupMembers(int slotIdx, int grpId, float delta)
{
	isPropagating = true;
    for (int i = 1; i <= 32; ++i)
    {
        if (i == slotIdx) continue;

        int otherGrpId = SlotStateHelpers::getGroupId(processor.apvts.state, i);
        int otherRoleId = SlotStateHelpers::getGroupRole(processor.apvts.state, i);

        if (otherGrpId == grpId && otherRoleId == 0) {
            float targetVol = lastVolume[i - 1] + delta;
            targetVol = juce::jlimit(-96.0f, 22.0f, targetVol);

            SlotStateHelpers::setParamUnnormalized(processor.apvts, SlotIDs::volume(i), targetVol);
            
            lastVolume[i - 1] = targetVol;
        }
    }
	isPropagating = false;
}

void LinkManager::handleMuteParameterChanged(const juce::String& parameterID, float newValue)
{
    int slotIdx = parameterID.substring(5).getIntValue();

    if (SlotStateHelpers::isGroupLeader(processor.apvts.state, slotIdx)) 
    {
        int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
        syncMutesWithinGroup(slotIdx, grpId, newValue);
    }
}

void LinkManager::syncMutesWithinGroup(int slotIdx, int grpId, float newValue)
{
	isPropagating = true;
    for (int i = 1; i <= 32; ++i) {
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