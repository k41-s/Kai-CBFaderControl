#pragma once
#include <JuceHeader.h>

namespace PresetTags
{
    inline const juce::Identifier StoresTreeType{ "Stores" };

    inline const juce::String RootXmlTag{ "KaiCBFaderControlData" };
    inline const juce::String StorePrefix{ "Store_" };

    inline const juce::String StoresButtonText{ "Stores" };
    inline const juce::String PresetsButtonText{ "Presets" };

    inline const juce::String ActiveSnapshotParamId{ "activeSnapshot" };
    inline const juce::String ActiveStoreParamId{ "activeStore" };
    inline const juce::String ActiveStoreParamName{ "Active Store" };

    inline const juce::Identifier StoreNameProp{ "name" };
    inline const juce::Identifier StorePinnedProp{ "isPinned" };
    inline const juce::Identifier StoreIdProp{ "storeId" };
    inline const juce::String DefaultStoreNamePrefix{ "ST " };

    inline const juce::Identifier VisibleStoresProp{ "visibleStores" };

    inline const juce::Identifier StoreSetsProp{ "StoreSets" };
    inline const juce::Identifier SetProp{ "Set" };
    inline const juce::Identifier SetNameProp{ "setName" };
    inline const juce::Identifier SetStoreIdsProp{ "storeIds" };
}

namespace PresetConstants
{
    inline const int noStore = 0;
    inline const int defaultStores = 8;
    inline const int maxStores = 127;

    inline const int maxStoreNameLength = 6;
    inline const int maxPinnedStores = 16;

    inline const juce::String storeIdDelimiter{ "," };
}

namespace ApvtsXmlTags
{
    inline const juce::String Param{ "PARAM" };
    inline const juce::Identifier Id{ "id" };
    inline const juce::Identifier Value{ "value" };
}