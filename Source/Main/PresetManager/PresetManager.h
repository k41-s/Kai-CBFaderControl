#pragma once
#include <JuceHeader.h>
#include "PresetConstants.h"

class PresetManager
{
public:
    PresetManager();
    ~PresetManager() = default;

    void saveStore(int index, const juce::ValueTree& currentState);
    juce::ValueTree getStore(int index) const;
    void clearStore(int index);

    void setStoreName(int index, const juce::String& newName);
    juce::String getStoreName(int index) const;

    void setStorePinned(int index, bool shouldPin);
    bool isStorePinned(int index) const;

    juce::Array<int> getPinnedStores() const;

    int getNumVisibleStores() const;
    void setNumVisibleStores(int num);

    std::unique_ptr<juce::XmlElement> createXml() const;
    void loadFromXml(juce::XmlElement* xml);

private:
    juce::String getStoreNodeName(int index) const;
    juce::String getDefaultStoreName(int index) const;
    juce::ValueTree getOrCreateStoreNode(int index);

    juce::ValueTree storesTree{ PresetTags::StoresTreeType };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};