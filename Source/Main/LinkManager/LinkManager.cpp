#include "LinkManager.h"
#include "../SlotIDs.h"
#include "../PluginProcessor/PluginProcessor.h"

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
    if (isUpdating) return;

    if (parameterID.startsWith(SlotIdStringPrefixes::vcaVolume)) 
        handleVcaVolumeParameterChanged(parameterID, newValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::vcaMute))
        handleVcaMuteParameterChanged(parameterID, newValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::volume)) 
        handleVolumeParameterChanged(parameterID, newValue);

    else if (parameterID.startsWith(SlotIdStringPrefixes::mute))
        handleMuteParameterChanged(parameterID, newValue);
}

void LinkManager::handleVcaVolumeParameterChanged(const juce::String& parameterID, float newValue)
{
    int grpIdx = parameterID.substring(10).getIntValue();
    float delta = newValue - lastVcaVolume[grpIdx - 1];
    lastVcaVolume[grpIdx - 1] = newValue;

    isUpdating = true;
    applyDeltaToGroupFromVca(grpIdx, delta);
    isUpdating = false;
}

void LinkManager::applyDeltaToGroupFromVca(int grpIdx, float delta)
{
    for (int i = 1; i <= 32; ++i) {
        int assignedGrp = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(i)), 0);
        if (assignedGrp == grpIdx) {
            float targetVol = lastVolume[i - 1] + delta;
            targetVol = juce::jlimit(-96.0f, 22.0f, targetVol);
            if (auto* param = processor.apvts.getParameter(SlotIDs::volume(i))) {
                param->setValueNotifyingHost(param->convertTo0to1(targetVol));
            }
            lastVolume[i - 1] = targetVol;
        }
    }
}

void LinkManager::handleVcaMuteParameterChanged(const juce::String& parameterID, float newValue)
{
    int grpIdx = parameterID.substring(8).getIntValue();
    isUpdating = true;
    syncGroupMutesWithVca(grpIdx, newValue);
    isUpdating = false;
}

void LinkManager::syncGroupMutesWithVca(int grpIdx, float newValue)
{
    for (int i = 1; i <= 32; ++i) {
        int assignedGrp = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(i)), 0);
        if (assignedGrp == grpIdx) 
        {
            if (auto* param = processor.apvts.getParameter(SlotIDs::mute(i))) 
            {
                param->setValueNotifyingHost(newValue);
            }
        }
    }
}

void LinkManager::handleVolumeParameterChanged(const juce::String& parameterID, float newValue)
{
    int slotIdx = parameterID.substring(7).getIntValue();
    int grpId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(slotIdx)), 0);
    int role = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupRole(slotIdx)), 0);

    float delta = newValue - lastVolume[slotIdx - 1];
    lastVolume[slotIdx - 1] = newValue;

    if (isSlotLeader(grpId, role))
    {
        isUpdating = true;
        applyDeltaToGroupMembers(slotIdx, grpId, delta);
        isUpdating = false;
    }
}

void LinkManager::applyDeltaToGroupMembers(int slotIdx, int grpId, float delta)
{
    for (int i = 1; i <= 32; ++i)
    {
        if (i == slotIdx) continue;

        int otherGrpId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(i)), 0);
        int otherRoleId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupRole(i)), 0);

        if (otherGrpId == grpId && otherRoleId == 0) {
            float targetVol = lastVolume[i - 1] + delta;
            targetVol = juce::jlimit(-96.0f, 22.0f, targetVol);

            if (auto* param = processor.apvts.getParameter(SlotIDs::volume(i)))
                param->setValueNotifyingHost(param->convertTo0to1(targetVol));
            
            lastVolume[i - 1] = targetVol;
        }
    }
}

void LinkManager::handleMuteParameterChanged(const juce::String& parameterID, float newValue)
{
    int slotIdx = parameterID.substring(5).getIntValue();
    int grpId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(slotIdx)), 0);
    int role = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupRole(slotIdx)), 0);

    if (isSlotLeader(grpId, role)) {
        isUpdating = true;
        syncMutesWithinGroup(slotIdx, grpId, newValue);
        isUpdating = false;
    }
}

void LinkManager::syncMutesWithinGroup(int slotIdx, int grpId, float newValue)
{
    for (int i = 1; i <= 32; ++i) {
        if (i == slotIdx) continue;

        int otherGrpId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(i)), 0);
        int otherRoleId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupRole(i)), 0);

        if (otherGrpId == grpId && otherRoleId == 0) 
        {
            if (auto* param = processor.apvts.getParameter(SlotIDs::mute(i))) 
                param->setValueNotifyingHost(newValue);
            
        }
    }
}

bool LinkManager::isSlotLeader(int grpId, int role)
{
    return grpId > 0 && role == 1;
}