#include "PresetManager.h"

PresetManager::PresetManager()
{
}

void PresetManager::saveSnapshot(int index, const juce::ValueTree& currentState)
{
	juce::String snapshotName = getSnapshotName(index);
    auto existingSnapshot = snapshotsTree.getChildWithName(snapshotName);

    auto clonedState = currentState.createCopy();

    if (existingSnapshot.isValid())
    {
        snapshotsTree.removeChild(existingSnapshot, nullptr);
    }

    juce::ValueTree newSnapshot(snapshotName);
    newSnapshot.addChild(clonedState, -1, nullptr);
    snapshotsTree.addChild(newSnapshot, -1, nullptr);
}

juce::ValueTree PresetManager::getSnapshot(int index) const
{
	juce::String snapshotName = getSnapshotName(index);
    auto snapshotNode = snapshotsTree.getChildWithName(snapshotName);

    if (snapshotNode.isValid() && snapshotNode.getNumChildren() > 0)
    {
        return snapshotNode.getChild(0);
    }

    return juce::ValueTree();
}

std::unique_ptr<juce::XmlElement> PresetManager::createXml() const
{
    return std::unique_ptr<juce::XmlElement>(snapshotsTree.createXml());
}

void PresetManager::loadFromXml(juce::XmlElement* xml)
{
    if (xml != nullptr && xml->hasTagName(PresetTags::SnapshotsTreeType.toString()))
    {
        snapshotsTree = juce::ValueTree::fromXml(*xml);
    }
}

juce::String PresetManager::getSnapshotName(int index) const
{
    return PresetTags::SnapshotPrefix + juce::String(index);
}
