#include "OscManager.h"
#include "../PluginProcessor/PluginProcessor.h"
#include "../../Utils/StateUtils/SlotStateHelpers.h"
#include "../SlotIDs.h"

OscManager::OscManager(KaiCBFaderControlAudioProcessor& p) 
    : processor(p), messageQueue(OscConstants::fifoSize, juce::OSCMessage("/empty"))
{
    init();
}

void OscManager::init()
{
    processor.apvts.state.addListener(this);

    receiver.addListener(this);

    startTimerHz(60);
}

OscManager::~OscManager()
{
	stopTimer();
    processor.apvts.state.removeListener(this);

    clearOscReceiver();
    clearOscSender();
}

void OscManager::clearOscReceiver()
{
    receiver.removeListener(this);
    receiver.disconnect();
}

void OscManager::clearOscSender()
{
    sender.disconnect();
}

void OscManager::connect()
{
    juce::String targetIp = SlotStateHelpers::getTargetIP(processor.apvts.state);
    int inPort = SlotStateHelpers::getIncomingPort(processor.apvts.state);
    int outPort = SlotStateHelpers::getOutgoingPort(processor.apvts.state);

    receiver.disconnect();
    sender.disconnect();

    connectOscReceiver(inPort);
    connectOscSender(targetIp, outPort);
}

void OscManager::connectOscReceiver(int inPort)
{
    if (receiver.connect(inPort))
        DBG("OSC Receiver connected on port: " << inPort);
    else
        DBG("FAILED to connect OSC Receiver on port: " << inPort);
}

void OscManager::connectOscSender(juce::String& targetIp, int& outPort)
{
    if (sender.connect(targetIp, outPort))
        DBG("OSC Sender connected to " << targetIp << ":" << outPort);
    else
        DBG("FAILED to connect OSC Sender to " << targetIp << ":" << outPort);
}

void OscManager::sendOSCMessage(const juce::OSCMessage& message)
{
    if (!sender.send(message))
        DBG("Failed to send OSC message to: " << message.getAddressPattern().toString());
}

void OscManager::oscBundleReceived(const juce::OSCBundle& bundle)
{
    for (auto& element : bundle)
    {
        if (element.isMessage())
            oscMessageReceived(element.getMessage());
        else if (element.isBundle())
            oscBundleReceived(element.getBundle());
    }
}

void OscManager::oscMessageReceived(const juce::OSCMessage& message)
{
    if (!message.getAddressPattern().toString().startsWith(OscConstants::prefix))
        return;

    pushMessageIntoFifoQueue(message);
}

void OscManager::pushMessageIntoFifoQueue(const juce::OSCMessage& message)
{
    auto writeHandle = messageFifo.write(1);

    if (writeHandle.blockSize1 > 0)
    {
        messageQueue[(size_t)writeHandle.startIndex1] = message;
    }
    else
    {
        DBG("OSC Message Queue is full! Dropping message.");
    }
}

void OscManager::timerCallback()
{
    int numReady = messageFifo.getNumReady();
    if (numReady == 0) return;

    auto readHandle = messageFifo.read(numReady);

    for (int i = 0; i < readHandle.blockSize1; ++i)
    {
        processQueuedMessage(messageQueue[(size_t)(readHandle.startIndex1 + i)]);
    }

    for (int i = 0; i < readHandle.blockSize2; ++i)
    {
        processQueuedMessage(messageQueue[(size_t)(readHandle.startIndex2 + i)]);
    }
}

void OscManager::processQueuedMessage(const juce::OSCMessage& message)
{
    juce::String address = message.getAddressPattern().toString();

    juce::StringArray parts;
    parts.addTokens(address, "/", "");

    if (parts.size() < OscConstants::Parsing::standardPathDepth) return;

    juce::String targetType = parts[2];
    int slotId = parts[3].getIntValue();
    juce::String paramType = parts[4];

    if (targetType == OscConstants::TargetTypes::fader() || targetType == OscConstants::TargetTypes::vca())
    {
        handleIncomingMessage(targetType, paramType, message, slotId);
    }
}

void OscManager::handleIncomingMessage(const juce::String& targetType, const juce::String& paramType, const juce::OSCMessage& message, int slotId)
{
    bool isVca = (targetType == OscConstants::TargetTypes::vca());

    if (paramType == OscConstants::ParamTypes::volume() && OscHelpers::isValidFloatMessage(message))
    {
        handleIncomingVolumeMessage(message, isVca, slotId);
    }
    else if (paramType == OscConstants::ParamTypes::mute() && OscHelpers::isValidIntMessage(message))
    {
        handleIncomingMuteMessage(message, isVca, slotId);
    }
    else if (paramType == OscConstants::ParamTypes::name() && OscHelpers::isValidStringMessage(message))
    {
        handleIncomingNameMessage(message, isVca, slotId);
    }
    else if (!isVca && paramType == OscConstants::ParamTypes::pan() && OscHelpers::isValidFloatMessage(message))
    {
        handleIncomingPanMessage(message, slotId);
    }
    else if (!isVca && paramType == OscConstants::ParamTypes::solo() && OscHelpers::isValidIntMessage(message))
    {
        handleIncomingSoloMessage(message, slotId);
    }
}

void OscManager::handleIncomingVolumeMessage(const juce::OSCMessage& message, bool isVca, int slotId)
{
    float incomingVolumeRaw = message[0].getFloat32();

    juce::String paramId = isVca ? SlotIDs::vcaVolume(slotId) : SlotIDs::volume(slotId);
    float currentVolumeRaw = SlotStateHelpers::getRawParamValue(processor.apvts, paramId);

    if (OscHelpers::volumeRawChanged(currentVolumeRaw, incomingVolumeRaw))
    {
        SlotStateHelpers::setParamUnnormalized(processor.apvts, paramId, incomingVolumeRaw);
    }
}

void OscManager::handleIncomingMuteMessage(const juce::OSCMessage& message, bool isVca, int slotId)
{
    bool isMuted = message[0].getInt32() != 0;
    juce::String paramId = isVca ? SlotIDs::vcaMute(slotId) : SlotIDs::mute(slotId);
    bool currentlyMuted = SlotStateHelpers::getRawParamValue(processor.apvts, paramId) > 0.5f;

    if (currentlyMuted != isMuted)
    {
        SlotStateHelpers::setParamNormalized(processor.apvts, paramId, isMuted ? 1.0f : 0.0f);
    }
}

void OscManager::handleIncomingNameMessage(const juce::OSCMessage& message, bool isVca, int slotId)
{
    juce::String incomingName = message[0].getString();

    juce::String currentName = isVca
        ? SlotStateHelpers::getVcaName(processor.apvts.state, slotId)
        : SlotStateHelpers::getSlotCustomName(processor.apvts.state, slotId);

    if (currentName != incomingName)
    {
        if (isVca)
            SlotStateHelpers::setVcaName(processor.apvts.state, slotId, incomingName);
        else
            SlotStateHelpers::setSlotCustomName(processor.apvts.state, slotId, incomingName);
    }
}

void OscManager::handleIncomingPanMessage(const juce::OSCMessage& message, int slotId)
{
    float incomingPanRaw = message[0].getFloat32();
    juce::String paramId = SlotIDs::pan(slotId);
    float currentPanRaw = SlotStateHelpers::getRawParamValue(processor.apvts, paramId);

    if (OscHelpers::panRawChanged(currentPanRaw, incomingPanRaw))
    {
        SlotStateHelpers::setParamUnnormalized(processor.apvts, paramId, incomingPanRaw);
    }
}

void OscManager::handleIncomingSoloMessage(const juce::OSCMessage& message, int slotId)
{
    bool isSoloed = message[0].getInt32() != 0;
    juce::String paramId = SlotIDs::solo(slotId);
    bool currentlySoloed = SlotStateHelpers::getRawParamValue(processor.apvts, paramId) > 0.5f;

    if (currentlySoloed != isSoloed)
    {
        SlotStateHelpers::setParamNormalized(processor.apvts, paramId, isSoloed ? 1.0f : 0.0f);
    }
}

void OscManager::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (property == SlotIDs::targetIP() ||
        property == SlotIDs::incomingPort() ||
        property == SlotIDs::outgoingPort())
    {
        connect();
    }
}