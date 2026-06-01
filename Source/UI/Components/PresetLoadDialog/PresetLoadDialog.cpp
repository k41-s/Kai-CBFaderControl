#include "PresetLoadDialog.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../Components/UIConstants.h"

PresetLoadDialog::PresetLoadDialog(std::function<void(bool, bool, bool)> onRecallCallback, std::function<void()> onCancelCallback)
    : onRecall(std::move(onRecallCallback)), onCancel(std::move(onCancelCallback))
{
    init();
}

void PresetLoadDialog::init()
{
    configComponents();
}

void PresetLoadDialog::configComponents()
{
    configTitleLabel();
    configToggles();
    configLoadBtn();
    configCancelBtn();
}

void PresetLoadDialog::configTitleLabel()
{
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(UISizeConstants::maxFontSize, juce::Font::bold));
    addAndMakeVisible(titleLabel);
}

void PresetLoadDialog::configToggles()
{
    layoutToggle.setToggleState(true, juce::dontSendNotification);
    dataToggle.setToggleState(true, juce::dontSendNotification);
    fullToggle.setToggleState(true, juce::dontSendNotification);

    addAndMakeVisible(layoutToggle);
    addAndMakeVisible(dataToggle);
    addAndMakeVisible(fullToggle);

    fullToggle.onClick = [this]
        {
            if (fullToggle.getToggleState())
            {
                layoutToggle.setToggleState(true, juce::dontSendNotification);
                dataToggle.setToggleState(true, juce::dontSendNotification);
            }
        };
}

void PresetLoadDialog::configLoadBtn()
{
    loadBtn.onClick = [this] {
        if (onRecall) {
            bool doLayout = layoutToggle.getToggleState() || fullToggle.getToggleState();
            bool doData = dataToggle.getToggleState() || fullToggle.getToggleState();
            bool doStores = fullToggle.getToggleState();
            onRecall(doLayout, doData, doStores);
        }
        };
    addAndMakeVisible(loadBtn);
}

void PresetLoadDialog::configCancelBtn()
{
    cancelBtn.onClick = [this] {
        if (onCancel) onCancel();
        };
    addAndMakeVisible(cancelBtn);
}

void PresetLoadDialog::paint(juce::Graphics& g)
{
    g.fillAll(MyColours::background);
}

void PresetLoadDialog::resized()
{
    auto area = getLocalBounds().reduced(20);

    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);

    layoutToggle.setBounds(area.removeFromTop(30));
    dataToggle.setBounds(area.removeFromTop(30));
    fullToggle.setBounds(area.removeFromTop(30));

    auto btnArea = area.removeFromBottom(40);
    cancelBtn.setBounds(btnArea.removeFromLeft(100));
    loadBtn.setBounds(btnArea.removeFromRight(100));
}