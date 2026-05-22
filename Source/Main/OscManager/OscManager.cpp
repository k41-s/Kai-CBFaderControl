#include "OscManager.h"
#include "../PluginProcessor/PluginProcessor.h"
#include "../../Utils/StateUtils/SlotStateHelpers.h"
#include "../SlotIDs.h"
#include "../../UI/Components/UIConstants.h"
#include "../../Utils/OSCUtils/OscHelpers.h"

OscManager::OscManager(KaiCBFaderControlAudioProcessor& p) 
    : processor(p), messageQueue(OscConstants::fifoSize, juce::OSCMessage("/empty"))
{
    init();
}

void OscManager::init()
{
    addListeners();
    startTimerHz(60);
}

void OscManager::addListeners()
{
    processor.apvts.state.addListener(this);
    receiver.addListener(this);
}

OscManager::~OscManager()
{
	stopTimer();

    removeListeners();
    clearOscReceiver();
    clearOscSender();
}

void OscManager::removeListeners()
{
    processor.apvts.state.removeListener(this);
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
    if (!message.getAddressPattern().toString().startsWith(OscConstants::incomingPrefix))
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
    if (numReady > 0)
    {
        auto readHandle = messageFifo.read(numReady);

        isProcessingQueue = true;

        for (int i = 0; i < readHandle.blockSize1; ++i)
            processQueuedMessage(messageQueue[(size_t)(readHandle.startIndex1 + i)]);

        for (int i = 0; i < readHandle.blockSize2; ++i)
            processQueuedMessage(messageQueue[(size_t)(readHandle.startIndex2 + i)]);

        isProcessingQueue = false;
    }

    pollAndBroadcastFaders();
}

bool OscManager::shouldBroadcastFloat(const juce::String& paramId, float newValue, const juce::String& paramType) const
{
    if (OscHelpers::isDuplicateFloat(lastReceivedOscFloats, paramId, newValue, paramType))
        return false;
    if (OscHelpers::isDuplicateFloat(lastSentOscFloats, paramId, newValue, paramType))
        return false;
    return true;
}

bool OscManager::shouldBroadcastInt(const juce::String& paramId, int newValue) const
{
    if (OscHelpers::isDuplicateValue(lastReceivedOscInts, paramId, newValue))
        return false;
    if (OscHelpers::isDuplicateValue(lastSentOscInts, paramId, newValue))
        return false;
    return true;
}

void OscManager::pollAndBroadcastFaders()
{
    juce::OSCBundle frameBundle;

    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        sendOscVolume(i, frameBundle);
        sendOscMute(i, frameBundle);
    }

    if (!frameBundle.isEmpty())
    {
        if (!sender.send(frameBundle))
            DBG("Failed to send OSC Bundle.");
    }
}

void OscManager::sendOscVolume(int i, juce::OSCBundle& frameBundle)
{
    juce::String volId = SlotIDs::volume(i);
    float currentVol = SlotStateHelpers::getRawParamValue(processor.apvts, volId);

    if (shouldBroadcastFloat(volId, currentVol, OscConstants::ParamTypes::volume()))
    {
        lastSentOscFloats[volId] = currentVol;
        lastReceivedOscFloats[volId] = currentVol;
        juce::String address = OscHelpers::buildAddress(OscConstants::TargetTypes::fader(), i, OscConstants::ParamTypes::volume());
        
        frameBundle.addElement(juce::OSCMessage(address, currentVol));
    }
}

void OscManager::sendOscMute(int i, juce::OSCBundle& frameBundle)
{
    juce::String muteId = SlotIDs::mute(i);
    int currentMute = SlotStateHelpers::getRawParamValue(processor.apvts, muteId) > 0.5f ? 1 : 0;

    if (shouldBroadcastInt(muteId, currentMute))
    {
        lastSentOscInts[muteId] = currentMute;
        lastReceivedOscInts[muteId] = currentMute;

        juce::String address = OscHelpers::buildAddress(OscConstants::TargetTypes::fader(), i, OscConstants::ParamTypes::mute());
        frameBundle.addElement(juce::OSCMessage(address, currentMute));
    }
}

void OscManager::broadcastNameChange(const juce::String& targetType, const juce::String& prefix, const juce::Identifier& property, juce::ValueTree& tree)
{
    int id = SlotStateHelpers::getIndexFromParamId(property.toString(), SlotIdStringPrefixes::slotName);
    juce::String newName = tree.getProperty(property).toString();

    juce::String dictKey = SlotIdStringPrefixes::slotName + juce::String(id);

    if (OscHelpers::isDuplicateValue(lastReceivedOscStrings, dictKey, newName) ||
        OscHelpers::isDuplicateValue(lastSentOscStrings, dictKey, newName))
        return;

    lastSentOscStrings[dictKey] = newName;
    lastReceivedOscStrings[dictKey] = newName;

    juce::String address = OscHelpers::buildAddress(OscConstants::TargetTypes::fader(), id, OscConstants::ParamTypes::name());
    sendOSCMessage(juce::OSCMessage(address, newName));
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

    if (targetType == OscConstants::TargetTypes::fader())
    {
        handleIncomingMessage(targetType, paramType, message, slotId);
    }
}

void OscManager::handleIncomingMessage(const juce::String& targetType, const juce::String& paramType, const juce::OSCMessage& message, int slotId)
{
    if (paramType == OscConstants::ParamTypes::volume() && OscHelpers::isValidFloatMessage(message))
        handleIncomingVolumeMessage(message, slotId);

    else if (paramType == OscConstants::ParamTypes::mute() && OscHelpers::isValidIntMessage(message))
        handleIncomingMuteMessage(message, slotId);

    else if (paramType == OscConstants::ParamTypes::name())
        handleIncomingNameMessage(message, slotId);
}

void OscManager::handleIncomingVolumeMessage(const juce::OSCMessage& message, int slotId)
{
    float incomingVolumeRaw = message[0].getFloat32();
    juce::String paramId = SlotIDs::volume(slotId);

    lastSentOscFloats[paramId] = incomingVolumeRaw;
    lastReceivedOscFloats[paramId] = incomingVolumeRaw;

    float currentVolumeRaw = SlotStateHelpers::getRawParamValue(processor.apvts, paramId);
    if (OscHelpers::volumeRawChanged(currentVolumeRaw, incomingVolumeRaw))
    {
        SlotStateHelpers::setParamUnnormalized(processor.apvts, paramId, incomingVolumeRaw);
    }
}

void OscManager::handleIncomingMuteMessage(const juce::OSCMessage& message, int slotId)
{
    bool isMuted = message[0].getInt32() != 0;
    int isMutedInt = isMuted ? 1 : 0;
    juce::String paramId = SlotIDs::mute(slotId);

    lastSentOscInts[paramId] = isMutedInt;
    lastReceivedOscInts[paramId] = isMutedInt;

    bool currentlyMuted = SlotStateHelpers::getRawParamValue(processor.apvts, paramId) > 0.5f;
    if (currentlyMuted != isMuted)
    {
        SlotStateHelpers::setParamNormalized(processor.apvts, paramId, isMuted ? 1.0f : 0.0f);
    }
}

void OscManager::handleIncomingNameMessage(const juce::OSCMessage& message, int slotId)
{
    juce::String rawName;

    if (message.size() > 0)
    {
        extractNameFromPayload(message, rawName);
    }

    if (rawName.isEmpty())
    {
        parseNameFromAddressPath(message, slotId, rawName);
    }

    juce::String sanitizedName;
    sanitizeString(rawName, sanitizedName);

	// Maybe make helper for this key generation
    juce::String dictKey = SlotIdStringPrefixes::slotName + juce::String(slotId);

    // Make helper for syncing these caches
    lastSentOscStrings[dictKey] = sanitizedName;
    lastReceivedOscStrings[dictKey] = sanitizedName;

    juce::String currentName = SlotStateHelpers::getSlotCustomName(processor.apvts.state, slotId);
    if (currentName != sanitizedName)
    {
        SlotStateHelpers::setSlotCustomName(processor.apvts.state, slotId, sanitizedName);
    }
}

void OscManager::extractNameFromPayload(const juce::OSCMessage& message, juce::String& rawName)
{
    if (message[0].isString())
        rawName = message[0].getString();
    else if (message[0].isInt32())
        rawName = juce::String(message[0].getInt32());
    else if (message[0].isFloat32())
        rawName = juce::String(message[0].getFloat32());
    else if (message[0].isBlob())
    {
        auto& blob = message[0].getBlob();
        rawName = juce::String::fromUTF8((const char*)blob.getData(), (int)blob.getSize());
    }
}

void OscManager::parseNameFromAddressPath(const juce::OSCMessage& message, int slotId, juce::String& rawName)
{
    juce::String address = message.getAddressPattern().toString();
    juce::String expectedPrefix = OscConstants::incomingPrefix + "/" +
        OscConstants::TargetTypes::fader() + "/" + juce::String(slotId) + "/" +
        OscConstants::ParamTypes::name() + "/";

    if (address.startsWithIgnoreCase(expectedPrefix))
        rawName = address.substring(expectedPrefix.length());
}

void OscManager::sanitizeString(juce::String& rawName, juce::String& sanitizedName)
{
    for (auto c : rawName)
    {
        if (c >= 32 && c <= 126)
            sanitizedName += juce::String::charToString(c);
    }

    sanitizedName = sanitizedName.trim();

    if (sanitizedName.length() > PluginConstants::maxSlotNameLength)
    {
        sanitizedName = sanitizedName.substring(0, PluginConstants::maxSlotNameLength);
    }
}

void OscManager::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    juce::String propStr = property.toString();

    if (property == SlotIDs::targetIP() ||
        property == SlotIDs::incomingPort() ||
        property == SlotIDs::outgoingPort())
    {
        connect();
    }
    else if (!isProcessingQueue)
    {
        if (propStr.startsWith(SlotIdStringPrefixes::slotName))
            broadcastNameChange(OscConstants::TargetTypes::fader(), SlotIdStringPrefixes::slotName, property, tree);
    }
}