#pragma once
#include <JuceHeader.h>
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../CustomLookAndFeel/BoxToggleLookAndFeel/BoxToggleLookAndFeel.h"
#include "../../../Utils/StateUtils/PresetHelpers.h"

class PresetLoadDialog : public juce::Component
{
public:
    PresetLoadDialog(std::function<void(const RecallScope&)> onRecallCallback, std::function<void()> onCancelCallback);
    ~PresetLoadDialog() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void init();

	// Config methods
    void configTogglesLists();
    void configComponents();
    void configTitleLabel();
    void configMainToggles();
    void configGranularToggles();
    void configLoadBtn();
    void setScopeStatesFromGranularToggles(RecallScope& scope);
    void configCancelBtn();

    void updateUIStates();
    void setAllGranularToggles(bool state);

	// Layout methods
    void setupMainToggles(juce::Rectangle<int>& area);
    void setupCustomGranularArea(juce::Rectangle<int>& area);
    void setupCustomAreaHeader(juce::Rectangle<int>& area);
    void setupGranularToggles(juce::Rectangle<int>& area);
    void setupBottomButtons(juce::Rectangle<int>& area);

    juce::Label titleLabel{ "", "Select Data to Load" };

    // Standard Options
    juce::ToggleButton layoutToggle{ "Layout Recall (Names, Colours, Ordering)" };
    juce::ToggleButton dataToggle{ "Data Recall Only (Volumes, Mutes, Pans, Snapshots)" };
    juce::ToggleButton fullToggle{ "Full Recall (Layout + Data)" };
    juce::ToggleButton customToggle{ "Custom Recall (Select precise elements below)" };

    // Custom Section
    juce::Label customLabel{ "", "Custom Recall Options:" };
    juce::ToggleButton volToggle{ "Volumes" };
    juce::ToggleButton muteToggle{ "Mutes" };
    juce::ToggleButton panToggle{ "Pans" };
    juce::ToggleButton soloToggle{ "Solos" };
    juce::ToggleButton vcaToggle{ "VCAs (Vol & Mute)" };

    juce::ToggleButton nameToggle{ "Names" };
    juce::ToggleButton colourToggle{ "Colours" };
    juce::ToggleButton routingToggle{ "Grouping & Stereo" };
    juce::ToggleButton activeToggle{ "Active States" };
    juce::ToggleButton storesToggle{ "Stores & Snapshots" };

    juce::Array<juce::ToggleButton*> mainToggles;
    juce::Array<juce::ToggleButton*> granularToggles;

    juce::TextButton selectAllBtn{ "Select All" };
    juce::TextButton deselectAllBtn{ "Deselect All" };

    juce::TextButton loadBtn{ "Load" };
    juce::TextButton cancelBtn{ "Cancel" };

    std::function<void(const RecallScope&)> onRecall;
    std::function<void()> onCancel;

    BoxToggleLookAndFeel boxToggleLF;

    float separatorY = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetLoadDialog)
};