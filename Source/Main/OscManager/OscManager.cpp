#include "OscManager.h"
#include "../PluginProcessor/PluginProcessor.h"
#include "../../Utils/StateUtils/SlotStateHelpers.h"
#include "../SlotIDs.h"

OscManager::OscManager(KaiCBFaderControlAudioProcessor& p) : processor(p)
{
    processor.apvts.state.addListener(this);

    receiver.addListener(this, OscAddressPatterns::root);
}

OscManager::~OscManager()
{
    processor.apvts.state.removeListener(this);
	receiver.removeListener(this);
    receiver.disconnect();
    sender.disconnect();
}

void OscManager::connect()
{
    juce::String targetIp = SlotStateHelpers::getTargetIP(processor.apvts.state);
    int inPort = SlotStateHelpers::getIncomingPort(processor.apvts.state);
    int outPort = SlotStateHelpers::getOutgoingPort(processor.apvts.state);

    receiver.disconnect();
    sender.disconnect();

    if (receiver.connect(inPort))
        DBG("OSC Receiver connected on port: " << inPort);
    else
        DBG("FAILED to connect OSC Receiver on port: " << inPort);

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

void OscManager::oscMessageReceived(const juce::OSCMessage& message)
{
    // Temporary debug output. 
    // In the next step, we will push this into the Lock-Free FIFO Buffer!
    DBG("Incoming OSC: " << message.getAddressPattern().toString());
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