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
    if (!message.getAddressPattern().toString().startsWith(OscAddressPatterns::prefix))
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
    // We are now safely on the GUI/Message Thread!
    DBG("Processed from Queue: " << message.getAddressPattern().toString());
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