#pragma once
#include <JuceHeader.h>

namespace DialogActions
{
    constexpr int Cancel = 0;
    constexpr int Confirm = 1;
    constexpr int Alternate = 2; // Used for a potential 3rd button
}

namespace DialogStrings
{
    // Button texts
    inline const juce::String SaveBtn = "Save";
    inline const juce::String CancelBtn = "Cancel";
    inline const juce::String UpdateBtn = "Update";
    inline const juce::String ResetBtn = "Reset";
    inline const juce::String CloseAnywayBtn = "Close anyway";

    // Titles
    inline const juce::String UnsavedTitle = "Unsaved Changes";
}

namespace PresetsMenuIds
{
    // Popup menu IDs for the Presets menu
    constexpr int LoadPreset = 1;
    constexpr int SavePreset = 2;
}