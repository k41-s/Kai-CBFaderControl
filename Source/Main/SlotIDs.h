#pragma once
#include <JuceHeader.h>

namespace SlotIdStringPrefixes
{
    static const juce::String isActive = "isActive_";
    static const juce::String slotName = "slotName_";
    static const juce::String volume = "volume_";
    static const juce::String mute = "mute_";
    static const juce::String pan = "pan_";
    static const juce::String solo = "solo_";
    static const juce::String soloSafe = "soloSafe_";
    static const juce::String isStereoLinked = "isStereoLinked_";
    static const juce::String isStereoMain = "isStereoMain_";
    static const juce::String linkedSlotId = "linkedSlotId_";
    static const juce::String groupId = "groupId_";
    static const juce::String groupRole = "groupRole_";
    static const juce::String isVcaExpanded = "isVcaExpanded_";
}

struct SlotIDs
{
    static juce::String isActive(int i) { return SlotIdStringPrefixes::isActive + juce::String(i); }
    static juce::String slotName(int i) { return SlotIdStringPrefixes::slotName + juce::String(i); }
    static juce::String volume(int i) { return SlotIdStringPrefixes::volume + juce::String(i); }
    static juce::String mute(int i) { return SlotIdStringPrefixes::mute + juce::String(i); }
    static juce::String pan(int i) { return SlotIdStringPrefixes::pan + juce::String(i); }
    static juce::String solo(int i) { return SlotIdStringPrefixes::solo + juce::String(i); }
    static juce::String soloSafe(int i) { return SlotIdStringPrefixes::soloSafe + juce::String(i); }

    static juce::String isStereoLinked(int i) { return SlotIdStringPrefixes::isStereoLinked + juce::String(i); }
    static juce::String isStereoMain(int i) { return SlotIdStringPrefixes::isStereoMain + juce::String(i); }
    static juce::String linkedSlotId(int i) { return SlotIdStringPrefixes::linkedSlotId + juce::String(i); }
    
    // 0 is no group
    static juce::String groupId(int i) { return SlotIdStringPrefixes::groupId + juce::String(i); }
    // 0 = Member, 1 = Leader, 2 = VCA Master
    static juce::String groupRole(int i) { return SlotIdStringPrefixes::groupRole + juce::String(i); }
    static juce::String isVcaExpanded(int g) { return SlotIdStringPrefixes::isVcaExpanded + juce::String(g); }

    static juce::Identifier targetIP() { return "targetIP"; }
    static juce::Identifier incomingPort() { return "incomingPort"; }
    static juce::Identifier outgoingPort() { return "outgoingPort"; }
};

namespace ParamSlotNameStringPrefixes
{
	static const juce::String isActive = "Is Active ";
    static const juce::String volume = "Volume ";
    static const juce::String mute = "Mute ";
    static const juce::String pan = "Pan ";
    static const juce::String solo = "Solo ";
    static const juce::String soloSafe = "Solo Safe ";
}

struct ParamSlotNames
{
    static juce::String isActive(int i) { return ParamSlotNameStringPrefixes::isActive + juce::String(i); }
    static juce::String volume(int i) { return ParamSlotNameStringPrefixes::volume + juce::String(i); }
    static juce::String mute(int i) { return ParamSlotNameStringPrefixes::mute + juce::String(i); }
    static juce::String pan(int i) { return ParamSlotNameStringPrefixes::pan + juce::String(i); }
    static juce::String solo(int i) { return ParamSlotNameStringPrefixes::solo + juce::String(i); }
    static juce::String soloSafe(int i) { return ParamSlotNameStringPrefixes::soloSafe + juce::String(i); }
};