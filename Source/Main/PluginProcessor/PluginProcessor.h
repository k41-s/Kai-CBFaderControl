/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../LinkManager/LinkManager.h"
#include "../../Utils/GlobalSlotRegistry/GlobalSlotRegistry.h"

//==============================================================================
/**
*/
class KaiCBFaderControlAudioProcessor :
    public juce::AudioProcessor,
    public juce::ChangeListener
{
public:
    //==============================================================================
    KaiCBFaderControlAudioProcessor();
    ~KaiCBFaderControlAudioProcessor() override;


    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


    //==============================================================================
    
    const juce::Uuid& getInstanceId() const { return instanceId; }

	void clearSlotRouting(int slotIdx);
	void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==============================================================================

    juce::AudioProcessorValueTreeState apvts;

	std::unique_ptr<LinkManager> linkManager;

    std::atomic<bool> isRestoringState{ false };

    juce::SharedResourcePointer<GlobalSlotRegistry> globalSlotRegistry;
private:
    void init();
    void initGlobalRegistry();
    void InitialiseNetworkingDefaults();
    void initLinkManager();

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void addParamsForSlot(juce::AudioProcessorValueTreeState::ParameterLayout& params, int i);
    void addParamsForVca(juce::AudioProcessorValueTreeState::ParameterLayout& params, int i);

    void removeFromGroup(juce::ValueTree& state, int slotIdx);
    void removeFromStereoPair(juce::ValueTree& state, int slotIdx);

    void claimActiveSlots() const;
    void releaseOwnedSlots() const;

    juce::Uuid instanceId;
    juce::Array<bool> wasSlotOwned;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KaiCBFaderControlAudioProcessor)
};
