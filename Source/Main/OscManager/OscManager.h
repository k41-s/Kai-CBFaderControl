#pragma once
#include <JuceHeader.h>
#include "OscConstants.h"
#include <unordered_map>

class KaiCBFaderControlAudioProcessor;

class OscManager : 
    public juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>,
    public juce::ValueTree::Listener,
    public juce::Timer
{
public:
    OscManager(KaiCBFaderControlAudioProcessor& p);
    ~OscManager() override;

    void connect();
    void sendOSCMessage(const juce::OSCMessage& message);

private:
    void init();
    void addListeners();
    void removeListeners();

    void connectOscReceiver(int inPort);
    void connectOscSender(juce::String& targetIp, int& outPort);
    void clearOscReceiver();
    void clearOscSender();

    void oscMessageReceived(const juce::OSCMessage& message) override;
    void oscBundleReceived(const juce::OSCBundle& bundle) override;
    void pushMessageIntoFifoQueue(const juce::OSCMessage& message);

    void timerCallback() override;
    bool shouldBroadcastFloat(const juce::String& paramId, float newValue, const juce::String& paramType);
    bool shouldBroadcastInt(const juce::String& paramId, int newValue);
    void pollAndBroadcastFaders();
    void sendOscVolume(int i);
    void sendOscMute(int i);

    void processQueuedMessage(const juce::OSCMessage& message);
    void handleIncomingMessage(const juce::String& targetType, const juce::String& paramType, const juce::OSCMessage& message, int slotId);
    void handleIncomingVolumeMessage(const juce::OSCMessage& message, int slotId);
    void handleIncomingMuteMessage(const juce::OSCMessage& message, int slotId);
    void handleIncomingNameMessage(const juce::OSCMessage& message, int slotId);

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;
    
    void handleRegularParametersChanged(const juce::String& parameterID, float newValue);
    void broadcastFloatParameter(const juce::String& targetType, const juce::String& paramType, const juce::String& prefix, const juce::String& parameterID, float newValue);
    void broadcastToggleParameter(const juce::String& targetType, const juce::String& paramType, const juce::String& prefix, const juce::String& parameterID, float newValue);
    void broadcastNameChange(const juce::String& targetType, const juce::String& prefix, const juce::Identifier& property, juce::ValueTree& tree);

    KaiCBFaderControlAudioProcessor& processor;
    juce::OSCReceiver receiver;
    juce::OSCSender sender;

    bool isProcessingQueue{ false };

    juce::AbstractFifo messageFifo{ OscConstants::fifoSize };
    std::vector<juce::OSCMessage> messageQueue;

    std::unordered_map<juce::String, float> lastReceivedOscFloats;
    std::unordered_map<juce::String, int> lastReceivedOscInts;
    std::unordered_map<juce::String, juce::String> lastReceivedOscStrings;

    std::unordered_map<juce::String, float> lastSentOscFloats;
    std::unordered_map<juce::String, int> lastSentOscInts;
    std::unordered_map<juce::String, juce::String> lastSentOscStrings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscManager)
};