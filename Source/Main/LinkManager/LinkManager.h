#pragma once
#include <JuceHeader.h>
#include "../../UI/Components/UIConstants.h"

class KaiCBFaderControlAudioProcessor; // Forward declaration to avoid circular dependency

class LinkManager : public juce::AudioProcessorValueTreeState::Listener
{
public:
    LinkManager(KaiCBFaderControlAudioProcessor& processor);
    ~LinkManager() override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    void init();

    void addRegularSlotListeners();
    void addVcaMasterListeners();
    void removeRegularSlotListeners();
    void removeVcaMasterListeners();
    void handleProcessorRestoringState(const juce::String& parameterID, float newValue);

	// Parameter Handlers
    void handleVcaVolumeParameterChanged(const juce::String& parameterID, float newValue);
    void handleVcaMuteParameterChanged(const juce::String& parameterID, float newValue);
    void handleVolumeParameterChanged(const juce::String& parameterID, float newValue);
    void handleMuteParameterChanged(const juce::String& parameterID, float newValue);
    void handleSoloParameterChanged(const juce::String& parameterID, float newValue);
    void handlePanParameterChanged(const juce::String& parameterID, float newValue);

	// Group Propagation Logic
    void applyDeltaToGroupFromVca(int grpIdx, float delta);
    void syncGroupMutesWithVca(int vcaIdx, float newValue);
    void applyDeltaToGroupMembers(int slotIdx, int grpId, float delta);
    void syncMutesWithinGroup(int slotIdx, int grpId, float newValue);
    void applyGroupVolumeDeltaToSlot(int slotIdx, float delta);

    // SIP Logic
    void updateSipState();
    void applyOrReleaseSIPMutes(bool anySoloActive);
    void handleNotCurrentlyMuted(bool isCurrentlyMuted, int i);
    void handleIsSipMuted(bool isSipMuted, int i);
    void handleSIP(int i, bool mute);

    // Custom Link Propagation Logic
    void propagateCustomLinkVolume(int sourceTrueId, bool isSourceVca, float delta);
    void propagateCustomLinkMute(int sourceTrueId, bool isSourceVca, float newValue);
    void propagateCustomLinkSolo(int sourceTrueId, bool isSourceVca, float newValue);
    void propagateCustomLinkPan(int sourceTrueId, bool isSourceVca, float newValue);

    // --- Array Wrappers ---
    float getLastVolume(int slotId) const { return lastVolume[slotId - 1]; }
    void setLastVolume(int slotId, float volume) { lastVolume[slotId - 1] = volume; }

    float getLastVcaVolume(int vcaId) const { return lastVcaVolume[vcaId - 1]; }
    void setLastVcaVolume(int vcaId, float volume) { lastVcaVolume[vcaId - 1] = volume; }

    float getUnclampedVolume(int slotId) const { return unclampedVolume[slotId - 1]; }
    void setUnclampedVolume(int slotId, float volume) { unclampedVolume[slotId - 1] = volume; }

    KaiCBFaderControlAudioProcessor& processor;

    std::array<float, PluginConstants::numSlots> lastVolume;
    std::array<float, PluginConstants::numSlots> unclampedVolume;
    std::array<float, PluginConstants::numVcas> lastVcaVolume;

    // Separate Atomic Locks
    std::atomic<bool> isPropagatingGroup{ false };
    std::atomic<bool> isPropagatingCustomLink{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LinkManager)
};