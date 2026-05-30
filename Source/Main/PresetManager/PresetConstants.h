#pragma once
#include <JuceHeader.h>

namespace PresetTags
{
    inline const juce::Identifier StoresTreeType{ "Stores" };

    inline const juce::String RootXmlTag{ "KaiCBFaderControlData" };
    inline const juce::String StorePrefix{ "Store_" };

    inline const juce::String StoresButtonText{ "Stores" };
    inline const juce::String PresetsButtonText{ "Presets" };

    inline const juce::String ActiveStoreParamId{ "activeStore" };
    inline const juce::String ActiveStoreParamName{ "Active Store" };

    inline const juce::Identifier StoreNameProp{ "name" };
    inline const juce::Identifier StorePinnedProp{ "isPinned" };
    inline const juce::Identifier StoreIdProp{ "storeId" };
    inline const juce::String DefaultStoreNamePrefix{ "Store " };

    inline const juce::Identifier VisibleStoresProp{ "visibleStores" };
}

namespace PresetConstants
{
    inline const int noStore = 0;
    inline const int defaultStores = 10;
    inline const int maxStores = 127;
}