#pragma once
#include <JuceHeader.h>

namespace PresetTags
{
    inline const juce::Identifier SnapshotsTreeType{ "Snapshots" };

    inline const juce::String RootXmlTag{ "KaiCBFaderControlData" };
    inline const juce::String SnapshotPrefix{ "Snapshot_" };

    inline const juce::String SaveButtonDefaultText{ "Save" };
    inline const juce::String PresetsButtonText{ "Presets" };

    inline const juce::String ModeLabelRecall{ "MODE: RECALL" };
    inline const juce::String ModeLabelSave{ "MODE: SAVE" };
}