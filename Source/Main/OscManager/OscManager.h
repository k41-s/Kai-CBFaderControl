#pragma once
#include <JuceHeader.h>
#include "OscConstants.h"
#include <unordered_map>

class CBFaderControlAudioProcessor;

class OscManager : 
    public juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>,
    public juce::ValueTree::Listener,
    public juce::Timer
{
public:
    OscManager(CBFaderControlAudioProcessor& p);
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
    void processFifoQueue();
    void handleConnectionLogic();
    void handlePingSending(juce::uint32 currentTime);
    void handleTimeoutThreshold(juce::uint32 currentTime);
	void setConnectionStatus(bool isConnected);
    void sendPing();
    void sendStartupRequest();

    bool shouldBroadcastFloat(const juce::String& paramId, float newValue, const juce::String& paramType) const;
    bool shouldBroadcastInt(const juce::String& paramId, int newValue) const;
    void pollAndBroadcastFaders();
    void sendOscVolume(int i, juce::OSCBundle& frameBundle);
    void sendOscMute(int i, juce::OSCBundle& frameBundle);
    void broadcastNameChange(const juce::String& targetType, const juce::String& prefix, const juce::Identifier& property, juce::ValueTree& tree);

    void processQueuedMessage(const juce::OSCMessage& message);
    void handleIncomingMessage(const juce::String& targetType, const juce::String& paramType, const juce::OSCMessage& message, int slotId);
    void handleIncomingVolumeMessage(const juce::OSCMessage& message, int slotId);
    void handleIncomingMuteMessage(const juce::OSCMessage& message, int slotId);
    void handleIncomingColourMessage(const juce::OSCMessage& message, int slotId);

    void handleIncomingStereoMessage(const juce::OSCMessage& message, int slotId);
    void removeExistingSlotLinks(int slotId);

    void handleIncomingNameMessage(const juce::OSCMessage& message, int slotId);
    void extractNameFromPayload(const juce::OSCMessage& message, juce::String& rawName);
    void parseNameFromAddressPath(const juce::OSCMessage& message, int slotId, juce::String& rawName);
    void sanitizeString(juce::String& rawName, juce::String& sanitizedName);

    void valueTreeRedirected(juce::ValueTree& tree) override;

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

    void syncFloatCaches(const juce::String& key, float value);
    void syncIntCaches(const juce::String& key, int value);
    void syncStringCaches(const juce::String& key, const juce::String& value);

    bool shouldProcessSlotOsc(int slotId) const;

    CBFaderControlAudioProcessor& processor;
    juce::OSCReceiver receiver;
    juce::OSCSender sender;

    std::atomic<bool> isProcessingQueue{ false };

    juce::AbstractFifo messageFifo{ OscConstants::fifoSize };
    std::vector<juce::OSCMessage> messageQueue;

    std::unordered_map<juce::String, float> lastReceivedOscFloats;
    std::unordered_map<juce::String, int> lastReceivedOscInts;
    std::unordered_map<juce::String, juce::String> lastReceivedOscStrings;

    std::unordered_map<juce::String, float> lastSentOscFloats;
    std::unordered_map<juce::String, int> lastSentOscInts;
    std::unordered_map<juce::String, juce::String> lastSentOscStrings;

    juce::uint32 lastPingSentTime = 0;
    juce::uint32 lastMessageReceivedTime = 0;
    bool currentlyConnected = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscManager)
};