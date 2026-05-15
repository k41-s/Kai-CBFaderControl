#pragma once
#include <JuceHeader.h>
#include "OscConstants.h"

class KaiCBFaderControlAudioProcessor; // Forward declaration

class OscManager : 
    public juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::RealtimeCallback>,
    public juce::ValueTree::Listener
{
public:
    OscManager(KaiCBFaderControlAudioProcessor& p);
    ~OscManager() override;

    void sendOSCMessage(const juce::OSCMessage& message);

    void connect();

private:
    void oscMessageReceived(const juce::OSCMessage& message) override;

    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    KaiCBFaderControlAudioProcessor& processor;
    juce::OSCSender sender;
    juce::OSCReceiver receiver;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscManager)
};