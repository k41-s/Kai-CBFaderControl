#pragma once
#include <JuceHeader.h>

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

    void handleVcaVolumeParameterChanged(const juce::String& parameterID, float newValue);
    void applyDeltaToGroupFromVca(int grpIdx, float delta);

    void handleVcaMuteParameterChanged(const juce::String& parameterID, float newValue);
    void syncGroupMutesWithVca(int vcaIdx, float newValue);
    
    void handleVolumeParameterChanged(const juce::String& parameterID, float newValue);
    void applyDeltaToGroupMembers(int slotIdx, int grpId, float delta);
    
    void handleMuteParameterChanged(const juce::String& parameterID, float newValue);
    void syncMutesWithinGroup(int slotIdx, int grpId, float newValue);

    bool isSlotLeader(int grpId, int role);

    KaiCBFaderControlAudioProcessor& processor;

    std::array<float, 32> lastVolume;
    std::array<float, 8> lastVcaVolume;

    std::atomic<bool> isUpdating{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LinkManager)
};