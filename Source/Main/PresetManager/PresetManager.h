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

    void setSnapshotName(int index, const juce::String& newName);
    juce::String getSnapshotName(int index) const;

    void setSnapshotPinned(int index, bool shouldPin);
    bool isSnapshotPinned(int index) const;

    juce::Array<int> getPinnedSnapshots() const;

    int getNumVisibleSnapshots() const;
    void setNumVisibleSnapshots(int num);

    std::unique_ptr<juce::XmlElement> createXml() const;
    void loadFromXml(juce::XmlElement* xml);

private:
    juce::String getSnapshotNodeName(int index) const;
    juce::String getDefaultSnapshotName(int index) const;
    juce::ValueTree getOrCreateSnapshotNode(int index);

    juce::ValueTree snapshotsTree{ PresetTags::SnapshotsTreeType };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};