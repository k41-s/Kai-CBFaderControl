#include "LinkManager.h"
#include "../SlotIDs.h"
#include "../PluginProcessor/PluginProcessor.h"

LinkManager::LinkManager(KaiCBFaderControlAudioProcessor& p) : processor(p)
{
    for (int i = 1; i <= 32; ++i) {
        processor.apvts.addParameterListener(SlotIDs::volume(i), this);
        processor.apvts.addParameterListener(SlotIDs::mute(i), this);

        lastVolume[i - 1] = processor.apvts.getRawParameterValue(SlotIDs::volume(i))->load();
    }
}

LinkManager::~LinkManager()
{
    for (int i = 1; i <= 32; ++i)
    {
        processor.apvts.removeParameterListener(SlotIDs::volume(i), this);
        processor.apvts.removeParameterListener(SlotIDs::mute(i), this);
    }
}

void LinkManager::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (isUpdating) return;

    if (parameterID.startsWith(SlotIdStringPrefixes::volume)) 
    {
        int slotIdx = parameterID.substring(7).getIntValue();
        int grpId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(slotIdx)), 0);
        int role = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupRole(slotIdx)), 0);

		float delta = newValue - lastVolume[slotIdx - 1];
		lastVolume[slotIdx - 1] = newValue;

        if (grpId > 0 && role > 0)
        {
            isUpdating = true;
            for (int i = 1; i <= 32; ++i)
            {
                if (i == slotIdx) continue;

                int otherGrpId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(i)), 0);
                int otherRoleId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupRole(i)), 0);

                if (otherGrpId == grpId && otherRoleId == 0) {
                    float targetVol = lastVolume[i - 1] + delta;
                    targetVol = juce::jlimit(-96.0f, 22.0f, targetVol);

                    if (auto* param = processor.apvts.getParameter(SlotIDs::volume(i))) {
                        param->setValueNotifyingHost(param->convertTo0to1(targetVol));
                    }
                    lastVolume[i - 1] = targetVol;
                }
            }
			isUpdating = false;
        }
    }
    else if (parameterID.startsWith(SlotIdStringPrefixes::mute))
    {
        int slotIdx = parameterID.substring(5).getIntValue();
        int grpId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(slotIdx)), 0);
        int role = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupRole(slotIdx)), 0);

        if (grpId > 0 && role > 0) {
            isUpdating = true;
            for (int i = 1; i <= 32; ++i) {
                if (i == slotIdx) continue;

                int otherGrpId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupId(i)), 0);
                int otherRoleId = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::groupRole(i)), 0);

                if (otherGrpId == grpId && otherRoleId == 0) {
                    if (auto* param = processor.apvts.getParameter(SlotIDs::mute(i))) {
                        param->setValueNotifyingHost(newValue);
                    }
                }
            }
            isUpdating = false;
        }
    }
}
