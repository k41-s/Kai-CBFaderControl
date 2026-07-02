#include "PluginProcessor.h"
#include "../PluginEditor/PluginEditor.h"
#include "../SlotIDs.h"
#include "../../Utils/StateUtils/SlotStateHelpers.h"
#include "../../UI/Components/UIConstants.h"

//==============================================================================
KaiCBFaderControlAudioProcessor::KaiCBFaderControlAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    ),
    undoManager(),
    apvts(*this, &undoManager, "Parameters", createParameterLayout())
{
    init();
}

void KaiCBFaderControlAudioProcessor::init()
{
    initListeners();
    initialiseNetworkingDefaults();
    initOscManager();
    initLinkManager();
    initPresetManager();
    initGlobalRegistry();
}

void KaiCBFaderControlAudioProcessor::initListeners()
{
    apvts.addParameterListener(PresetTags::ActiveStoreParamId, this);
}

void KaiCBFaderControlAudioProcessor::initialiseNetworkingDefaults()
{
    auto& state = apvts.state;
    if (!state.hasProperty(SlotIDs::targetIP()))
        SlotStateHelpers::setTargetIP(state, NetworkConstants::defaultTargetIp);

    if (!state.hasProperty(SlotIDs::incomingPort()))
        SlotStateHelpers::setIncomingPort(state, NetworkConstants::defaultIncomingPort);

    if (!state.hasProperty(SlotIDs::outgoingPort()))
        SlotStateHelpers::setOutgoingPort(state, NetworkConstants::defaultOutgoingPort);
}

void KaiCBFaderControlAudioProcessor::initOscManager()
{
    oscManager = std::make_unique<OscManager>(*this);
    oscManager->connect();
}

void KaiCBFaderControlAudioProcessor::initLinkManager()
{
    linkManager = std::make_unique<LinkManager>(*this);
}

void KaiCBFaderControlAudioProcessor::initPresetManager()
{
    presetManager = std::make_unique<PresetManager>();
}

void KaiCBFaderControlAudioProcessor::initGlobalRegistry()
{
    wasSlotOwned.insertMultiple(0, false, PluginConstants::numSlots);

    globalSlotRegistry->addChangeListener(this);
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
    {
        bool isLocallyActive = SlotStateHelpers::isSlotActive(apvts, i);
        bool isOwned = (globalSlotRegistry->getSlotMode(i, getInstanceId(), isLocallyActive) == SlotMode::FullAccess);

        setWasSlotOwned(i, isOwned);
    }
}

KaiCBFaderControlAudioProcessor::~KaiCBFaderControlAudioProcessor()
{
    removeListeners();
    clearGlobalSlotRegistry();
}

void KaiCBFaderControlAudioProcessor::removeListeners()
{
    apvts.removeParameterListener(PresetTags::ActiveStoreParamId, this);
}

void KaiCBFaderControlAudioProcessor::clearGlobalSlotRegistry()
{
    globalSlotRegistry->removeChangeListener(this);
    releaseOwnedSlots();
}

void KaiCBFaderControlAudioProcessor::releaseOwnedSlots() const
{
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
        globalSlotRegistry->releaseSlot(i, getInstanceId());
}

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout KaiCBFaderControlAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;

    for (int i = 1; i <= PluginConstants::numSlots; ++i)
        addParamsForSlot(params, i);
    for (int i = 1; i <= PluginConstants::numVcas; ++i)
        addParamsForVca(params, i);

    addActiveStoreParam(params);

    return params;
}

void KaiCBFaderControlAudioProcessor::addParamsForSlot(juce::AudioProcessorValueTreeState::ParameterLayout& params, int i)
{
    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::isActive(i),
        ParamSlotNames::isActive(i),
        false
    ));
    params.add(std::make_unique<juce::AudioParameterFloat>(
        SlotIDs::volume(i),
        ParamSlotNames::volume(i),
        juce::NormalisableRange<float>(
            PluginConstants::volumeMin,
            PluginConstants::volumeMax,
            PluginConstants::fineRes,
            1.0f),
        0.0f
    ));
    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::mute(i),
        ParamSlotNames::mute(i),
        false
    ));
    params.add(std::make_unique<juce::AudioParameterFloat>(
        SlotIDs::pan(i),
        ParamSlotNames::pan(i),
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.0f),
        0.0f
    ));
    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::solo(i),
        ParamSlotNames::solo(i),
        false
    ));
    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::soloSafe(i),
        ParamSlotNames::soloSafe(i),
        false
    ));
}

void KaiCBFaderControlAudioProcessor::addParamsForVca(juce::AudioProcessorValueTreeState::ParameterLayout& params, int i)
{
    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::vcaEnabled(i), ParamSlotNames::vcaEnabled(i), false));

    params.add(std::make_unique<juce::AudioParameterFloat>(
        SlotIDs::vcaVolume(i),
        ParamSlotNames::vcaVolume(i),
        juce::NormalisableRange<float>(
            PluginConstants::volumeMin,
            PluginConstants::volumeMax,
            PluginConstants::fineRes,
            1.0f),
        0.0f
    ));
    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::vcaMute(i), ParamSlotNames::vcaMute(i), false));

    params.add(std::make_unique<juce::AudioParameterBool>(
        SlotIDs::isVcaExpanded(i), ParamSlotNames::isVcaExpanded(i), true));
}

void KaiCBFaderControlAudioProcessor::addActiveStoreParam(juce::AudioProcessorValueTreeState::ParameterLayout& params)
{
    params.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(PresetTags::ActiveStoreParamId, 1),
        PresetTags::ActiveStoreParamName,
        0, PresetConstants::maxStores, PresetConstants::noStore
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

void KaiCBFaderControlAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String KaiCBFaderControlAudioProcessor::getProgramName(int index)
{
    return {};
}

void KaiCBFaderControlAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void KaiCBFaderControlAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
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
bool KaiCBFaderControlAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}
#endif

void KaiCBFaderControlAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

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
    return new KaiCBFaderControlAudioProcessorEditor(*this);
}

//==============================================================================

void KaiCBFaderControlAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::XmlElement parentXml(PresetTags::RootXmlTag);

    parentXml.setAttribute(PluginConstants::xmlEditorWidthAttribute, lastEditorWidth);
    parentXml.setAttribute(PluginConstants::xmlEditorHeightAttribute, lastEditorHeight);

    saveApvtsState(parentXml);
    saveStoreState(parentXml);
    copyXmlToBinary(parentXml, destData);
}

void KaiCBFaderControlAudioProcessor::saveApvtsState(juce::XmlElement& parentXml)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> apvtsXml(state.createXml());
    if (apvtsXml != nullptr)
        parentXml.addChildElement(apvtsXml.release());
}

void KaiCBFaderControlAudioProcessor::saveStoreState(juce::XmlElement& parentXml) const
{
    std::unique_ptr<juce::XmlElement> storesXml(presetManager->createXml());
    if (storesXml != nullptr)
        parentXml.addChildElement(storesXml.release());
}

void KaiCBFaderControlAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    isRestoringState = true;
    std::unique_ptr<juce::XmlElement> parentXml(getXmlFromBinary(data, sizeInBytes));

    if (parentXml.get() != nullptr)
    {
        if (parentXml->hasTagName(PresetTags::RootXmlTag))
        {
            lastEditorWidth = parentXml->getIntAttribute(PluginConstants::xmlEditorWidthAttribute, -1);
            lastEditorHeight = parentXml->getIntAttribute(PluginConstants::xmlEditorHeightAttribute, -1);

            restoreApvts(parentXml);
            restoreStores(parentXml);

            int activeStoreId = SlotStateHelpers::getActiveStoreId(apvts);
            if (activeStoreId > 0)
            {
                auto storeState = presetManager->getStore(activeStoreId);

                if (storeState.isValid())
                {
                    apvts.replaceState(storeState.createCopy());
                }
            }
        }
        else if (parentXml->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*parentXml));
        }
    }

    initialiseNetworkingDefaults();
    claimActiveSlots();
    isRestoringState = false;
}

void KaiCBFaderControlAudioProcessor::restoreApvts(std::unique_ptr<juce::XmlElement>& parentXml)
{
    auto* apvtsXml = parentXml->getChildByName(apvts.state.getType());
    if (apvtsXml != nullptr)
        apvts.replaceState(juce::ValueTree::fromXml(*apvtsXml));
}

void KaiCBFaderControlAudioProcessor::restoreStores(std::unique_ptr<juce::XmlElement>& parentXml) const
{
    auto* storesXml = parentXml->getChildByName(PresetTags::StoresTreeType.toString());
    if (storesXml != nullptr)
        presetManager->loadFromXml(storesXml);
}

void KaiCBFaderControlAudioProcessor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &globalSlotRegistry.get())
    {
        for (int i = 1; i <= PluginConstants::numSlots; ++i)
        {
            bool isLocallyActive = SlotStateHelpers::isSlotActive(apvts, i);
            bool isCurrentlyOwned = (globalSlotRegistry->getSlotMode(i, getInstanceId(), isLocallyActive) == SlotMode::FullAccess);

            if (getWasSlotOwned(i) && !isCurrentlyOwned)
            {
                clearSlotRouting(i);
            }
            setWasSlotOwned(i, isCurrentlyOwned);
        }
    }
}

void KaiCBFaderControlAudioProcessor::clearSlotRouting(int slotIdx)
{
    auto& state = apvts.state;
    int linkedIdx = SlotStateHelpers::getLinkedSlotId(state, slotIdx);

    removeFromGroup(state, slotIdx);
    removeFromStereoPair(state, slotIdx);

    if (linkedIdx != -1)
    {
        removeFromStereoPair(state, linkedIdx);
    }
}

void KaiCBFaderControlAudioProcessor::removeFromGroup(juce::ValueTree& state, int slotIdx)
{
    SlotStateHelpers::setGroupId(state, slotIdx, 0);
    SlotStateHelpers::setGroupRole(state, slotIdx, GroupRole::Member);
}

void KaiCBFaderControlAudioProcessor::removeFromStereoPair(juce::ValueTree& state, int slotIdx)
{
	SlotStateHelpers::unlinkStereoSlot(state, slotIdx);
}

void KaiCBFaderControlAudioProcessor::claimActiveSlots() const
{
    for (int i = 1; i <= PluginConstants::numSlots; ++i)
        if (SlotStateHelpers::isSlotActive(apvts, i))
            globalSlotRegistry->claimSlot(i, getInstanceId());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KaiCBFaderControlAudioProcessor();
}

void KaiCBFaderControlAudioProcessor::forceRecallStore(int storeIdx)
{
    if (storeIdx > 0)
    {
        juce::MessageManager::callAsync([this, storeIdx]()
            {
                auto storeState = presetManager->getStore(storeIdx);
                if (storeState.isValid())
                {
                    isRestoringState = true;
                    apvts.replaceState(storeState.createCopy());
                    isRestoringState = false;
                }
            });
    }
}

void KaiCBFaderControlAudioProcessor::savePresetToFile(const juce::File& file)
{
    juce::XmlElement parentXml(PresetTags::RootXmlTag);
    saveApvtsState(parentXml);
    saveStoreState(parentXml);
    parentXml.writeTo(file);
}

std::unique_ptr<juce::XmlElement> KaiCBFaderControlAudioProcessor::loadPresetFile(const juce::File& file)
{
    return juce::XmlDocument::parse(file);
}

void KaiCBFaderControlAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == PresetTags::ActiveStoreParamId)
    {
        handleActiveStoreParameterChanged(newValue);
    }
}

void KaiCBFaderControlAudioProcessor::handleActiveStoreParameterChanged(float newValue)
{
    int storeIdx = juce::roundToInt(newValue);

    if (storeIdx > 0 && !isRestoringState)
    {
        juce::MessageManager::callAsync([this, storeIdx]()
            {
                auto storeState = presetManager->getStore(storeIdx);
                if (storeState.isValid())
                {
                    isRestoringState = true;
                    apvts.replaceState(storeState.createCopy());
                    isRestoringState = false;
                }
            });
    }
}