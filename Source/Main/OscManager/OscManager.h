#pragma once
#include <JuceHeader.h>
#include "OscConstants.h"

class KaiCBFaderControlAudioProcessor; // Forward declaration

class OscManager : 
    public juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>,
    public juce::ValueTree::Listener,
    public juce::Timer,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    OscManager(KaiCBFaderControlAudioProcessor& p);
    ~OscManager() override;

    void sendOSCMessage(const juce::OSCMessage& message);

    void connect();

private:
    void init();
    void addListeners();
    void addRegularSlotListeners();
    void addVcaListeners();

    void removeListeners();
    void removeRegularSlotListeners();
    void removeVcaListeners();

    void clearOscReceiver();
    void clearOscSender();

    void connectOscReceiver(int inPort);
    void connectOscSender(juce::String& targetIp, int& outPort);

    void oscMessageReceived(const juce::OSCMessage& message) override;
    void pushMessageIntoFifoQueue(const juce::OSCMessage& message);

    void oscBundleReceived(const juce::OSCBundle& bundle) override;

    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    void timerCallback() override;
    void processQueuedMessage(const juce::OSCMessage& message);

    void handleIncomingMessage(const juce::String& targetType, const juce::String& paramType, const juce::OSCMessage& message, int slotId);
    void handleIncomingVolumeMessage(const juce::OSCMessage& message, bool isVca, int slotId);
    void handleIncomingMuteMessage(const juce::OSCMessage& message, bool isVca, int slotId);
    void handleIncomingNameMessage(const juce::OSCMessage& message, bool isVca, int slotId);
    void handleIncomingPanMessage(const juce::OSCMessage& message, int slotId);
    void handleIncomingSoloMessage(const juce::OSCMessage& message, int slotId);

    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void handleRegularParametersChanged(const juce::String& parameterID, float newValue);
    void handleVcaParametersChanged(const juce::String& parameterID, float newValue);

    void broadcastFloatParameter(const juce::String& targetType, const juce::String& paramType, const juce::String& prefix, const juce::String& parameterID, float newValue);
    void broadcastToggleParameter(const juce::String& targetType, const juce::String& paramType, const juce::String& prefix, const juce::String& parameterID, float newValue);
    void broadcastNameChange(const juce::String& targetType, const juce::String& prefix, const juce::Identifier& property, juce::ValueTree& tree);

    KaiCBFaderControlAudioProcessor& processor;
    juce::OSCSender sender;
    juce::OSCReceiver receiver;

    juce::AbstractFifo messageFifo{ OscConstants::fifoSize };
    std::vector<juce::OSCMessage> messageQueue;

    bool isProcessingQueue{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscManager)
};