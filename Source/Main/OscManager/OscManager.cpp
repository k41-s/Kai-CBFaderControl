#include "OscManager.h"
#include "../PluginProcessor/PluginProcessor.h"
#include "../SlotIDs.h"
#include "../../UI/Components/UIConstants.h"
#include "../../Utils/StateUtils/SlotStateHelpers.h"
#include "../../Utils/OSCUtils/OscHelpers.h"
#include "../../Utils/ScopedAtomicSetter.h"

OscManager::OscManager(CBFaderControlAudioProcessor& p) 
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
	handleConnectionLogic();
    processFifoQueue();
    pollAndBroadcastFaders();
}

void OscManager::processFifoQueue()
{
    int numReady = messageFifo.getNumReady();
    if (numReady > 0)
    {
        auto readHandle = messageFifo.read(numReady);

        ScopedAtomicSetter setter(isProcessingQueue, true);

        for (int i = 0; i < readHandle.blockSize1; ++i)
            processQueuedMessage(messageQueue[(size_t)(readHandle.startIndex1 + i)]);

        for (int i = 0; i < readHandle.blockSize2; ++i)
            processQueuedMessage(messageQueue[(size_t)(readHandle.startIndex2 + i)]);
    }
}

void OscManager::handleConnectionLogic()
{
    juce::uint32 currentTime = juce::Time::getMillisecondCounter();
    handlePingSending(currentTime);
    handleTimeoutThreshold(currentTime);
}

void OscManager::handlePingSending(juce::uint32 currentTime)
{
    if (OscHelpers::shouldSendPing(currentTime, lastMessageReceivedTime, lastPingSentTime))
    {
        sendPing();
        lastPingSentTime = currentTime;
    }
}

void OscManager::handleTimeoutThreshold(juce::uint32 currentTime)
{
    if (currentlyConnected && (currentTime - lastMessageReceivedTime >= OscConstants::timeoutThreshold))
    {
		setConnectionStatus(false);
        DBG("OSC Connection Lost (Timeout due to silence)");
    }
}

void OscManager::setConnectionStatus(bool isConnected)
{
	currentlyConnected = isConnected;
	SlotStateHelpers::setPluginConnected(processor.apvts.state, isConnected);
}

void OscManager::sendPing()
{
    juce::String address = OscConstants::outgoingPrefix + "/" + OscConstants::SystemCommands::ping();
    sendOSCMessage(juce::OSCMessage(address));
}

void OscManager::sendStartupRequest()
{
    juce::String address = OscConstants::outgoingPrefix + "/" + OscConstants::SystemCommands::startup();
    sendOSCMessage(juce::OSCMessage(address));
    DBG("Sent Startup Request");
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
	if (!currentlyConnected) return; // Can remove this if needed for testing

    juce::OSCBundle frameBundle;

    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        if (shouldProcessSlotOsc(i))
        {
            sendOscVolume(i, frameBundle);
            sendOscMute(i, frameBundle);
        }
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
        syncFloatCaches(volId, currentVol);
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
        syncIntCaches(muteId, currentMute);
        juce::String address = OscHelpers::buildAddress(OscConstants::TargetTypes::fader(), i, OscConstants::ParamTypes::mute());
        frameBundle.addElement(juce::OSCMessage(address, currentMute));
    }
}

void OscManager::broadcastNameChange(const juce::String& targetType, const juce::String& prefix, const juce::Identifier& property, juce::ValueTree& tree)
{
    int id = SlotStateHelpers::getIndexFromParamId(property.toString(), SlotIdStringPrefixes::slotName);
    juce::String newName = tree.getProperty(property).toString();
    juce::String dictKey = SlotIDs::slotName(id);

    if (OscHelpers::isDuplicateValue(lastReceivedOscStrings, dictKey, newName) ||
        OscHelpers::isDuplicateValue(lastSentOscStrings, dictKey, newName))
        return;

    syncStringCaches(dictKey, newName);

    juce::String address = OscHelpers::buildAddress(OscConstants::TargetTypes::fader(), id, OscConstants::ParamTypes::name());
    sendOSCMessage(juce::OSCMessage(address, newName));
}

void OscManager::processQueuedMessage(const juce::OSCMessage& message)
{
    lastMessageReceivedTime = juce::Time::getMillisecondCounter();

    if (!currentlyConnected)
    {
		setConnectionStatus(true);
        DBG("OSC Connection Established / Restored");
        sendStartupRequest();
    }

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

    else if (paramType == OscConstants::ParamTypes::colour() && OscHelpers::isValidStringMessage(message))
        handleIncomingColourMessage(message, slotId);

    else if (paramType == OscConstants::ParamTypes::stereo() && OscHelpers::isValidIntMessage(message))
        handleIncomingStereoMessage(message, slotId);

    else if (paramType == OscConstants::ParamTypes::name())
        handleIncomingNameMessage(message, slotId);
}

void OscManager::handleIncomingVolumeMessage(const juce::OSCMessage& message, int slotId)
{
    float incomingVolumeRaw = message[0].getFloat32();
    juce::String paramId = SlotIDs::volume(slotId);

    syncFloatCaches(paramId, incomingVolumeRaw);

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

    syncIntCaches(paramId, isMutedInt);

    bool currentlyMuted = SlotStateHelpers::getRawParamValue(processor.apvts, paramId) > 0.5f;
    if (currentlyMuted != isMuted)
    {
        SlotStateHelpers::setParamNormalized(processor.apvts, paramId, isMuted ? 1.0f : 0.0f);
    }
}

void OscManager::handleIncomingColourMessage(const juce::OSCMessage& message, int slotId)
{
    juce::String incomingHexColour = message[0].getString();
    juce::String currentColour = SlotStateHelpers::getSlotColour(processor.apvts.state, slotId);

    if (currentColour != incomingHexColour)
    {
        SlotStateHelpers::setSlotColour(processor.apvts.state, slotId, incomingHexColour);
    }
}

void OscManager::handleIncomingStereoMessage(const juce::OSCMessage& message, int slotId)
{
    bool isStereo = message[0].getInt32() != 0;
    bool currentlyStereo = SlotStateHelpers::isXpStereo(processor.apvts.state, slotId);

    if (currentlyStereo != isStereo)
    {
        SlotStateHelpers::setXpStereo(processor.apvts.state, slotId, isStereo);

        if (isStereo && SlotStateHelpers::isStereoLinked(processor.apvts.state, slotId))
        {
            removeExistingSlotLinks(slotId);
        }
    }
}

void OscManager::removeExistingSlotLinks(int slotId)
{
    int linkedIdx = SlotStateHelpers::getLinkedSlotId(processor.apvts.state, slotId);

    SlotStateHelpers::unlinkStereoSlot(processor.apvts.state, slotId);
    if (linkedIdx != -1)
    {
        SlotStateHelpers::unlinkStereoSlot(processor.apvts.state, linkedIdx);
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

    juce::String dictKey = SlotIDs::slotName(slotId);

    syncStringCaches(dictKey, sanitizedName);

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

void OscManager::valueTreeRedirected(juce::ValueTree& tree)
{
    connect();

    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        broadcastNameChange(OscConstants::TargetTypes::fader(), 
            SlotIdStringPrefixes::slotName,
            juce::Identifier(SlotIDs::slotName(i)), 
            tree
        );
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

void OscManager::syncFloatCaches(const juce::String& key, float value)
{
    lastSentOscFloats[key] = value;
    lastReceivedOscFloats[key] = value;
}

void OscManager::syncIntCaches(const juce::String& key, int value) 
{
    lastSentOscInts[key] = value;
    lastReceivedOscInts[key] = value;
}

void OscManager::syncStringCaches(const juce::String& key, const juce::String& value) 
{
    lastSentOscStrings[key] = value;
    lastReceivedOscStrings[key] = value;
}

bool OscManager::shouldProcessSlotOsc(int slotId) const
{
    bool isLocallyActive = SlotStateHelpers::isSlotActive(processor.apvts, slotId);
    SlotMode mode = processor.globalSlotRegistry->getSlotMode(slotId, processor.getInstanceId(), isLocallyActive);

    if (mode == SlotMode::FullAccess)
        return true;

    if (mode == SlotMode::Disabled && processor.globalSlotRegistry->isSlotUnowned(slotId))
        return true;

    return false;
}