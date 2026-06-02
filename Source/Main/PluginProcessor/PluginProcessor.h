#pragma once
#include <JuceHeader.h>
#include "../LinkManager/LinkManager.h"
#include "../OscManager/OscManager.h"
#include "../PresetManager/PresetManager.h"
#include "../PresetManager/PresetConstants.h"
#include "../../Utils/GlobalSlotRegistry/GlobalSlotRegistry.h"

//==============================================================================
class KaiCBFaderControlAudioProcessor :
    public juce::AudioProcessor,
    public juce::ChangeListener,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    KaiCBFaderControlAudioProcessor();
    ~KaiCBFaderControlAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================

    const juce::Uuid& getInstanceId() const { return instanceId; }

    void clearSlotRouting(int slotIdx);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==============================================================================

    void forceRecallStore(int storeIdx);

    //=============================================================================
    
    void savePresetToFile(const juce::File& file);
    std::unique_ptr<juce::XmlElement> loadPresetFile(const juce::File& file);

    //=============================================================================

    void parameterChanged(const juce::String& parameterID, float newValue) override;

    //==============================================================================

    juce::AudioProcessorValueTreeState apvts;

    std::unique_ptr<LinkManager> linkManager;
    std::unique_ptr<OscManager> oscManager;
    std::unique_ptr<PresetManager> presetManager;

    std::atomic<bool> isRestoringState{ false };

    juce::SharedResourcePointer<GlobalSlotRegistry> globalSlotRegistry;

    int lastEditorWidth = -1;
    int lastEditorHeight = -1;

private:
    void init();
    void initListeners();
    void initialiseNetworkingDefaults();
    void initOscManager();
    void initLinkManager();
    void initPresetManager();
    void initGlobalRegistry();

    void removeListeners();
    void clearGlobalSlotRegistry();

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void addParamsForSlot(juce::AudioProcessorValueTreeState::ParameterLayout& params, int i);
    void addParamsForVca(juce::AudioProcessorValueTreeState::ParameterLayout& params, int i);
    void addActiveStoreParam(juce::AudioProcessorValueTreeState::ParameterLayout& params);

    void removeFromGroup(juce::ValueTree& state, int slotIdx);
    void removeFromStereoPair(juce::ValueTree& state, int slotIdx);

    void claimActiveSlots() const;
    void releaseOwnedSlots() const;

    bool getWasSlotOwned(int slotId) const { return wasSlotOwned[slotId - 1]; }
    void setWasSlotOwned(int slotId, bool isOwned) { wasSlotOwned.set(slotId - 1, isOwned); }

    void saveApvtsState(juce::XmlElement& parentXml);
    void saveStoreState(juce::XmlElement& parentXml) const;

    void restoreApvts(std::unique_ptr<juce::XmlElement>& parentXml);
    void restoreStores(std::unique_ptr<juce::XmlElement>& parentXml) const;

    void handleActiveStoreParameterChanged(float newValue);

    juce::Uuid instanceId;
    juce::Array<bool> wasSlotOwned;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KaiCBFaderControlAudioProcessor)
};