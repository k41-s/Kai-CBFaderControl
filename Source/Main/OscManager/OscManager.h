#pragma once
#include <JuceHeader.h>
#include "OscConstants.h"

class KaiCBFaderControlAudioProcessor; // Forward declaration

class OscManager : 
    public juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>,
    public juce::ValueTree::Listener,
    public juce::Timer
{
public:
    OscManager(KaiCBFaderControlAudioProcessor& p);
    ~OscManager() override;

    void sendOSCMessage(const juce::OSCMessage& message);

    void connect();

private:
    void init();

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

    KaiCBFaderControlAudioProcessor& processor;
    juce::OSCSender sender;
    juce::OSCReceiver receiver;

    juce::AbstractFifo messageFifo{ OscConstants::fifoSize };
    std::vector<juce::OSCMessage> messageQueue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscManager)
};