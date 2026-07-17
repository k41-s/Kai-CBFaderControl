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

void PresetManager::clearStore(int index)
{
    juce::String nodeName = getStoreNodeName(index);
    auto node = storesTree.getChildWithName(nodeName);

    if (node.isValid())
    {
        node.removeAllChildren(nullptr);

        node.removeProperty(PresetTags::StoreNameProp, nullptr);

        node.setProperty(PresetTags::StorePinnedProp, false, nullptr);
    }
}

void PresetManager::setStoreName(int index, const juce::String& newName)
{
    auto node = getOrCreateStoreNode(index);
    juce::String finalName = newName.substring(0, PresetConstants::maxStoreNameLength).trim();

    if (finalName.isEmpty() || finalName == getDefaultStoreName(index))
    {
        node.removeProperty(PresetTags::StoreNameProp, nullptr);
    }
    else
    {
        node.setProperty(PresetTags::StoreNameProp, finalName, nullptr);
    }
}

juce::String PresetManager::getStoreName(int index) const
{
    juce::String nodeName = getStoreNodeName(index);
    auto node = storesTree.getChildWithName(nodeName);

    if (node.isValid() && node.hasProperty(PresetTags::StoreNameProp))
    {
        juce::String savedName = node.getProperty(PresetTags::StoreNameProp).toString().trim();
        if (savedName.isNotEmpty())
            return savedName;
    }

    return getDefaultStoreName(index);
}

void PresetManager::setStorePinned(int index, bool shouldPin)
{
    if (shouldPin && !isStorePinned(index))
    {
        if (getPinnedStores().size() >= PresetConstants::maxPinnedStores)
        {
            return;
        }
    }

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
    for (int i = 1; i <= PresetConstants::numStores; ++i)
    {
        if (isStorePinned(i))
            pinned.add(i);
    }
    return pinned;
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
    juce::String numStr = juce::String(index);
    juce::String prefix = PresetTags::DefaultStoreNamePrefix;

    if ((prefix + numStr).length() > PresetConstants::maxStoreNameLength)
    {
        prefix = prefix.trim();
    }

    return (prefix + numStr).substring(0, PresetConstants::maxStoreNameLength);
}

juce::ValueTree PresetManager::getOrCreateStoreNode(int index)
{
    juce::String nodeName = getStoreNodeName(index);
    auto node = storesTree.getChildWithName(nodeName);

    if (!node.isValid())
    {
        node = juce::ValueTree(nodeName);

        bool isPinnedByDefault = (index >= 1 && index <= PresetConstants::defaultPinnedStores);
        node.setProperty(PresetTags::StorePinnedProp, isPinnedByDefault, nullptr);
        storesTree.addChild(node, -1, nullptr);
    }
    return node;
}

juce::ValueTree PresetManager::getOrCreateStoreSetsNode()
{
    auto setsNode = storesTree.getChildWithName(PresetTags::StoreSetsProp);
    if (!setsNode.isValid())
    {
        setsNode = juce::ValueTree(PresetTags::StoreSetsProp);
        storesTree.addChild(setsNode, -1, nullptr);
    }
    return setsNode;
}

void PresetManager::saveStoreSet(const juce::String& setName, const juce::Array<int>& pinnedStores)
{
    auto setsNode = getOrCreateStoreSetsNode();

    juce::ValueTree setNode = setsNode.getChildWithProperty(PresetTags::SetNameProp, setName);
    if (!setNode.isValid())
    {
        setNode = juce::ValueTree(PresetTags::SetProp);
        setNode.setProperty(PresetTags::SetNameProp, setName, nullptr);
        setsNode.addChild(setNode, -1, nullptr);
    }

    juce::StringArray strArray;
    for (int id : pinnedStores)
        strArray.add(juce::String(id));

    setNode.setProperty(PresetTags::SetStoreIdsProp, strArray.joinIntoString(PresetConstants::storeIdDelimiter), nullptr);
}

void PresetManager::removeStoreSet(const juce::String& setName)
{
    auto setsNode = storesTree.getChildWithName(PresetTags::StoreSetsProp);
    if (setsNode.isValid())
    {
        auto setNode = setsNode.getChildWithProperty(PresetTags::SetNameProp, setName);
        if (setNode.isValid())
            setsNode.removeChild(setNode, nullptr);
    }
}

juce::StringArray PresetManager::getStoreSetNames() const
{
    juce::StringArray names;
    auto setsNode = storesTree.getChildWithName(PresetTags::StoreSetsProp);
    if (setsNode.isValid())
    {
        for (auto child : setsNode)
        {
            if (child.hasProperty(PresetTags::SetNameProp))
                names.add(child.getProperty(PresetTags::SetNameProp).toString());
        }
    }
    return names;
}

juce::Array<int> PresetManager::getStoresInSet(const juce::String& setName) const
{
    juce::Array<int> stores;
    auto setsNode = storesTree.getChildWithName(PresetTags::StoreSetsProp);
    if (setsNode.isValid())
    {
        auto setNode = setsNode.getChildWithProperty(PresetTags::SetNameProp, setName);
        if (setNode.isValid() && setNode.hasProperty(PresetTags::SetStoreIdsProp))
        {
            juce::String idsString = setNode.getProperty(PresetTags::SetStoreIdsProp).toString();
            juce::StringArray idStrs = juce::StringArray::fromTokens(idsString, PresetConstants::storeIdDelimiter, "");
            for (const auto& idStr : idStrs)
            {
                stores.add(idStr.getIntValue());
            }
        }
    }
    return stores;
}