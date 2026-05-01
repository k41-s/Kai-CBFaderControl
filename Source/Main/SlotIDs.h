#pragma once
#include <JuceHeader.h>

namespace SlotIdStringPrefixes
{
	// Regular slot parameters
    static const juce::String isActive = "isActive_";
    static const juce::String slotName = "slotName_";
    static const juce::String volume = "volume_";
    static const juce::String mute = "mute_";
    static const juce::String pan = "pan_";
    static const juce::String solo = "solo_";
    static const juce::String soloSafe = "soloSafe_";

	// Stereo linking parameters
    static const juce::String isStereoLinked = "isStereoLinked_";
    static const juce::String isStereoMain = "isStereoMain_";
    static const juce::String linkedSlotId = "linkedSlotId_";

	// Grouping parameters
    static const juce::String groupId = "groupId_";
    static const juce::String groupRole = "groupRole_";
    static const juce::String groupColour = "groupColour_";

	// VCA parameters
    static const juce::String vcaEnabled = "vcaEnabled_";
    static const juce::String vcaVolume = "vcaVolume_";
    static const juce::String vcaMute = "vcaMute_";
    static const juce::String vcaName = "vcaName_";
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
    
    static juce::String groupId(int i) { return SlotIdStringPrefixes::groupId + juce::String(i); }
    static juce::String groupRole(int i) { return SlotIdStringPrefixes::groupRole + juce::String(i); }
    static juce::String groupColour(int i) { return SlotIdStringPrefixes::groupColour + juce::String(i); }

    static juce::String vcaEnabled(int grp) { return SlotIdStringPrefixes::vcaEnabled + juce::String(grp); }
    static juce::String vcaVolume(int vca) { return SlotIdStringPrefixes::vcaVolume + juce::String(vca); }
    static juce::String vcaMute(int vca) { return SlotIdStringPrefixes::vcaMute + juce::String(vca); }
    static juce::String vcaName(int vca) { return SlotIdStringPrefixes::vcaName + juce::String(vca); }
    static juce::String isVcaExpanded(int vca) { return SlotIdStringPrefixes::isVcaExpanded + juce::String(vca); }

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

	static const juce::String vcaEnabled = "VCA Enabled ";
    static const juce::String vcaVolume = "VCA Volume ";
    static const juce::String vcaMute = "VCA Mute ";
    static const juce::String isVcaExpanded = "VCA Expanded ";
}

struct ParamSlotNames
{
    static juce::String isActive(int i) { return ParamSlotNameStringPrefixes::isActive + juce::String(i); }
    static juce::String volume(int i) { return ParamSlotNameStringPrefixes::volume + juce::String(i); }
    static juce::String mute(int i) { return ParamSlotNameStringPrefixes::mute + juce::String(i); }
    static juce::String pan(int i) { return ParamSlotNameStringPrefixes::pan + juce::String(i); }
    static juce::String solo(int i) { return ParamSlotNameStringPrefixes::solo + juce::String(i); }
    static juce::String soloSafe(int i) { return ParamSlotNameStringPrefixes::soloSafe + juce::String(i); }

    static juce::String vcaEnabled(int i) { return ParamSlotNameStringPrefixes::vcaEnabled + juce::String(i); }
    static juce::String vcaVolume(int i) { return ParamSlotNameStringPrefixes::vcaVolume + juce::String(i); }
    static juce::String vcaMute(int i) { return ParamSlotNameStringPrefixes::vcaMute + juce::String(i); }
    static juce::String isVcaExpanded(int i) { return ParamSlotNameStringPrefixes::isVcaExpanded + juce::String(i); }
};