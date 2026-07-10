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

    struct LinkRouteDetails
    {
        int sourceTreeId = 0;
        int targetId = 0;
        bool isTargetVca = false;
    };

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

    float getUnclampedPan(int slotId) const { return unclampedPan[slotId - 1]; }
    void setUnclampedPan(int slotId, float pan) { unclampedPan[slotId - 1] = pan; }

    float getUnclampedVcaVolume(int vcaId) const { return unclampedVcaVolume[vcaId - 1]; }
    void setUnclampedVcaVolume(int vcaId, float volume) { unclampedVcaVolume[vcaId - 1] = volume; }

    float getLastPan(int slotId) const { return lastPan[slotId - 1]; }
    void setLastPan(int slotId, float pan) { lastPan[slotId - 1] = pan; }

    LinkRouteDetails getLinkRouteDetails(int sourceTrueId, bool isSourceVca) const;
	void applyVolumeDelta(int targetId, bool isVca, float delta);
	void applyPanDelta(int targetId, float delta);
    float applyPolarity(float delta, int sourceTreeId) const;

    void forEachGroupMember(int leaderSlotIdx, int grpId, std::function<void(int)> action) const;

    KaiCBFaderControlAudioProcessor& processor;

    std::array<float, PluginConstants::numSlots> lastVolume;
    std::array<float, PluginConstants::numSlots> unclampedVolume;
    std::array<float, PluginConstants::numSlots> lastPan;
    std::array<float, PluginConstants::numSlots> unclampedPan;
    std::array<float, PluginConstants::numVcas> lastVcaVolume;
    std::array<float, PluginConstants::numVcas> unclampedVcaVolume;

    // Separate Atomic Locks
    std::atomic<bool> isPropagatingGroup{ false };
    std::atomic<bool> isPropagatingCustomLink{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LinkManager)
};