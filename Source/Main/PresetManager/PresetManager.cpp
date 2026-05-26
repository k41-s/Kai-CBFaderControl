#include "PresetManager.h"

PresetManager::PresetManager()
{
}

void PresetManager::saveSnapshot(int index, const juce::ValueTree& currentState)
{
    auto node = getOrCreateSnapshotNode(index);
    auto clonedState = currentState.createCopy();

    if (node.getNumChildren() > 0)
    {
        node.removeChild(0, nullptr);
    }

    node.addChild(clonedState, 0, nullptr);
}

juce::ValueTree PresetManager::getSnapshot(int index) const
{
    juce::String nodeName = getSnapshotNodeName(index);
    auto snapshotNode = snapshotsTree.getChildWithName(nodeName);

    if (snapshotNode.isValid() && snapshotNode.getNumChildren() > 0)
        return snapshotNode.getChild(0);

    return juce::ValueTree();
}

void PresetManager::setSnapshotName(int index, const juce::String& newName)
{
    auto node = getOrCreateSnapshotNode(index);
    node.setProperty(PresetTags::SnapshotNameProp, newName, nullptr);
}

juce::String PresetManager::getSnapshotName(int index) const
{
    juce::String nodeName = getSnapshotNodeName(index);
    auto node = snapshotsTree.getChildWithName(nodeName);

    if (node.isValid() && node.hasProperty(PresetTags::SnapshotNameProp))
        return node.getProperty(PresetTags::SnapshotNameProp);

    return getDefaultSnapshotName(index);
}

void PresetManager::setSnapshotPinned(int index, bool shouldPin)
{
    auto node = getOrCreateSnapshotNode(index);
    node.setProperty(PresetTags::SnapshotPinnedProp, shouldPin, nullptr);
}

bool PresetManager::isSnapshotPinned(int index) const
{
    juce::String nodeName = getSnapshotNodeName(index);
    auto node = snapshotsTree.getChildWithName(nodeName);

    if (node.isValid() && node.hasProperty(PresetTags::SnapshotPinnedProp))
        return node.getProperty(PresetTags::SnapshotPinnedProp);

    return false;
}

juce::Array<int> PresetManager::getPinnedSnapshots() const
{
    juce::Array<int> pinned;
    for (int i = 1; i <= 127; ++i)
    {
        if (isSnapshotPinned(i))
            pinned.add(i);
    }
    return pinned;
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

juce::String PresetManager::getSnapshotNodeName(int index) const
{
    return PresetTags::SnapshotPrefix + juce::String(index);
}

juce::String PresetManager::getDefaultSnapshotName(int index) const
{
    return PresetTags::DefaultSnapshotNamePrefix + juce::String(index);
}

juce::ValueTree PresetManager::getOrCreateSnapshotNode(int index)
{
    juce::String nodeName = getSnapshotNodeName(index);
    auto node = snapshotsTree.getChildWithName(nodeName);

    if (!node.isValid())
    {
        node = juce::ValueTree(nodeName);
        node.setProperty(PresetTags::SnapshotNameProp, getDefaultSnapshotName(index), nullptr);
        node.setProperty(PresetTags::SnapshotPinnedProp, false, nullptr);
        snapshotsTree.addChild(node, -1, nullptr);
    }
    return node;
}