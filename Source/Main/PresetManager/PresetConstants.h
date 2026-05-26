#pragma once
#include <JuceHeader.h>

namespace PresetTags
{
    inline const juce::Identifier SnapshotsTreeType{ "Snapshots" };

    inline const juce::String RootXmlTag{ "KaiCBFaderControlData" };
    inline const juce::String SnapshotPrefix{ "Snapshot_" };

    // Different btn text
    inline const juce::String SaveButtonDefaultText{ "Save" };
    inline const juce::String PresetsButtonText{ "Presets" };

    // probably will remove these
    inline const juce::String ModeLabelRecall{ "MODE: RECALL" };
    inline const juce::String ModeLabelSave{ "MODE: SAVE" };

    inline const juce::String ActiveSnapshotParamId{ "activeSnapshot" };
    inline const juce::String ActiveSnapshotParamName{ "Active Snapshot" };

    inline const juce::Identifier SnapshotNameProp{ "name" };
    inline const juce::Identifier SnapshotPinnedProp{ "isPinned" };
    inline const juce::String DefaultSnapshotNamePrefix{ "Snapshot " };
}