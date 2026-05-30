#include "PresetManager.h"

PresetManager::PresetManager()
{
}

void PresetManager::saveStore(int index, const juce::ValueTree& currentState)
{
    auto node = getOrCreateStoreNode(index);
    auto clonedState = currentState.createCopy();

    if (node.getNumChildren() > 0)
    {
        node.removeChild(0, nullptr);
    }

    node.addChild(clonedState, 0, nullptr);
}

juce::ValueTree PresetManager::getStore(int index) const
{
    juce::String nodeName = getStoreNodeName(index);
    auto storeNode = storesTree.getChildWithName(nodeName);

    if (storeNode.isValid() && storeNode.getNumChildren() > 0)
        return storeNode.getChild(0);

    return juce::ValueTree();
}

void PresetManager::setStoreName(int index, const juce::String& newName)
{
    auto node = getOrCreateStoreNode(index);
    node.setProperty(PresetTags::StoreNameProp, newName, nullptr);
}

juce::String PresetManager::getStoreName(int index) const
{
    juce::String nodeName = getStoreNodeName(index);
    auto node = storesTree.getChildWithName(nodeName);

    if (node.isValid() && node.hasProperty(PresetTags::StoreNameProp))
        return node.getProperty(PresetTags::StoreNameProp);

    return getDefaultStoreName(index);
}

void PresetManager::setStorePinned(int index, bool shouldPin)
{
    auto node = getOrCreateStoreNode(index);
    node.setProperty(PresetTags::StorePinnedProp, shouldPin, nullptr);
}

bool PresetManager::isStorePinned(int index) const
{
    juce::String nodeName = getStoreNodeName(index);
    auto node = storesTree.getChildWithName(nodeName);

    if (node.isValid() && node.hasProperty(PresetTags::StorePinnedProp))
        return node.getProperty(PresetTags::StorePinnedProp);

    return false;
}

juce::Array<int> PresetManager::getPinnedStores() const
{
    juce::Array<int> pinned;
    for (int i = 1; i <= PresetConstants::maxStores; ++i)
    {
        if (isStorePinned(i))
            pinned.add(i);
    }
    return pinned;
}

int PresetManager::getNumVisibleStores() const
{
    if (storesTree.hasProperty(PresetTags::VisibleStoresProp))
        return storesTree.getProperty(PresetTags::VisibleStoresProp);

    return PresetConstants::defaultStores;
}

void PresetManager::setNumVisibleStores(int num)
{
    int safeNum = juce::jlimit(1, PresetConstants::maxStores, num);
    storesTree.setProperty(PresetTags::VisibleStoresProp, safeNum, nullptr);
}

std::unique_ptr<juce::XmlElement> PresetManager::createXml() const
{
    return std::unique_ptr<juce::XmlElement>(storesTree.createXml());
}

void PresetManager::loadFromXml(juce::XmlElement* xml)
{
    if (xml != nullptr && xml->hasTagName(PresetTags::StoresTreeType.toString()))
    {
        storesTree = juce::ValueTree::fromXml(*xml);
    }
}

juce::String PresetManager::getStoreNodeName(int index) const
{
    return PresetTags::StorePrefix + juce::String(index);
}

juce::String PresetManager::getDefaultStoreName(int index) const
{
    return PresetTags::DefaultStoreNamePrefix + juce::String(index);
}

juce::ValueTree PresetManager::getOrCreateStoreNode(int index)
{
    juce::String nodeName = getStoreNodeName(index);
    auto node = storesTree.getChildWithName(nodeName);

    if (!node.isValid())
    {
        node = juce::ValueTree(nodeName);
        node.setProperty(PresetTags::StoreNameProp, getDefaultStoreName(index), nullptr);
        node.setProperty(PresetTags::StorePinnedProp, false, nullptr);
        storesTree.addChild(node, -1, nullptr);
    }
    return node;
}