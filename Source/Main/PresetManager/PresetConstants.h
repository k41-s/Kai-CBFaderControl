#pragma once
#include <JuceHeader.h>

namespace PresetTags
{
    inline const juce::Identifier SnapshotsTreeType{ "Snapshots" };

    inline const juce::String RootXmlTag{ "KaiCBFaderControlData" };
    inline const juce::String SnapshotPrefix{ "Snapshot_" };

    inline const juce::String StoresButtonText{ "Stores" };
    inline const juce::String PresetsButtonText{ "Presets" };

    // probably will remove these
    inline const juce::String ModeLabelRecall{ "MODE: RECALL" };
    inline const juce::String ModeLabelSave{ "MODE: SAVE" };

    inline const juce::String ActiveSnapshotParamId{ "activeSnapshot" };
    inline const juce::String ActiveSnapshotParamName{ "Active Snapshot" };

    inline const juce::Identifier SnapshotNameProp{ "name" };
    inline const juce::Identifier SnapshotPinnedProp{ "isPinned" };
    inline const juce::String DefaultSnapshotNamePrefix{ "Snapshot " };

    inline const juce::Identifier VisibleSnapshotsProp{ "visibleSnapshots" };
}

namespace PresetConstants
{
	inline const int noSnapshot = 0;
	inline const int defaultSnapshots = 10;
	inline const int maxSnapshots = 127;
}