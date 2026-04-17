/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "../PluginEditor/PluginEditor.h"
#include "../SlotIDs.h"

//==============================================================================
KaiCBFaderControlAudioProcessor::KaiCBFaderControlAudioProcessor()
     : AudioProcessor (BusesProperties()
         .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
     ),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    InitialiseNetworkingDefaults();
    fillIsActiveParamsList();
}

void KaiCBFaderControlAudioProcessor::fillIsActiveParamsList()
{
    for (int i = 0; i < 32; ++i) {
        isActiveParams[i] = apvts.getRawParameterValue(SlotIDs::isActive(i + 1));
		jassert(isActiveParams[i] != nullptr);
    }
}

void KaiCBFaderControlAudioProcessor::InitialiseNetworkingDefaults()
{
    auto& state = apvts.state;

    if (!state.hasProperty(SlotIDs::targetIP()))
        state.setProperty(SlotIDs::targetIP(), "127.0.0.1", nullptr);

    if (!state.hasProperty(SlotIDs::incomingPort()))
        state.setProperty(SlotIDs::incomingPort(), 8000, nullptr);

    if (!state.hasProperty(SlotIDs::outgoingPort()))
        state.setProperty(SlotIDs::outgoingPort(), 8001, nullptr);
}

KaiCBFaderControlAudioProcessor::~KaiCBFaderControlAudioProcessor()
{
}

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout KaiCBFaderControlAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;

    for (int i = 1; i <= 32; ++i) {
        addParamsForSlot(params, i);
    }
    return params;
}

void KaiCBFaderControlAudioProcessor::addParamsForSlot(juce::AudioProcessorValueTreeState::ParameterLayout& params, int i)
{
    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::isActive(i),
        "Is Active " + juce::String(i),
        false
    ));
    params.add(std::make_unique<juce::AudioParameterFloat>(
        SlotIDs::volume(i),
        "Volume " + juce::String(i),
        juce::NormalisableRange<float>(-96.0f, 22.0f, 0.25f, 1.0f),
        0.0f
    ));
    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::mute(i),
        "Mute " + juce::String(i),
        false
    ));
    params.add(std::make_unique<juce::AudioParameterFloat>(
        SlotIDs::pan(i),
        "Pan " + juce::String(i),
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.0f),
        0.0f
    ));
    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::solo(i),
        "Solo " + juce::String(i),
        false
    ));
    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::soloSafe(i),
        "Solo Safe " + juce::String(i),
        false
    ));
}

//==============================================================================
const juce::String KaiCBFaderControlAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KaiCBFaderControlAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KaiCBFaderControlAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KaiCBFaderControlAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KaiCBFaderControlAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KaiCBFaderControlAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int KaiCBFaderControlAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KaiCBFaderControlAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String KaiCBFaderControlAudioProcessor::getProgramName (int index)
{
    return {};
}

void KaiCBFaderControlAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void KaiCBFaderControlAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void KaiCBFaderControlAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KaiCBFaderControlAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}
#endif

void KaiCBFaderControlAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool KaiCBFaderControlAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* KaiCBFaderControlAudioProcessor::createEditor()
{
    return new KaiCBFaderControlAudioProcessorEditor (*this);
}

//==============================================================================
void KaiCBFaderControlAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void KaiCBFaderControlAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KaiCBFaderControlAudioProcessor();
}
