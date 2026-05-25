#pragma once
#include <JuceHeader.h>
#include "PresetConstants.h"

class PresetManager
{
public:
    PresetManager();
    ~PresetManager() = default;

    void saveSnapshot(int index, const juce::ValueTree& currentState);
    juce::ValueTree getSnapshot(int index) const;

    std::unique_ptr<juce::XmlElement> createXml() const;
    void loadFromXml(juce::XmlElement* xml);

private:
    juce::String getSnapshotName(int index) const;

    juce::ValueTree snapshotsTree{ PresetTags::SnapshotsTreeType };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};