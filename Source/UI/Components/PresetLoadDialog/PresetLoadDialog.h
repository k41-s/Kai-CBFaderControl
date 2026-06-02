#pragma once
#include <JuceHeader.h>

class PresetLoadDialog : public juce::Component
{
public:
    PresetLoadDialog(std::function<void(bool, bool, bool)> onRecallCallback, std::function<void()> onCancelCallback);
    ~PresetLoadDialog() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void init();

	// Component Configuration
    void configComponents();
    void configTitleLabel();
    void configToggles();
    void configLoadBtn();
    void configCancelBtn();


    juce::Label titleLabel{ "", "Select Data to Load" };

    juce::ToggleButton layoutToggle{ "Layout Recall (Names, Colours, Ordering)" };
    juce::ToggleButton dataToggle{ "Data Recall Only (Volumes, Mutes, Pans, Snapshots)" };
    juce::ToggleButton fullToggle{ "Full Recall (Layout + Data)" };

    juce::TextButton loadBtn{ "Load" };
    juce::TextButton cancelBtn{ "Cancel" };

    std::function<void(bool, bool, bool)> onRecall;
    std::function<void()> onCancel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetLoadDialog)
};