#include "PresetLoadDialog.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../Components/UIConstants.h"

PresetLoadDialog::PresetLoadDialog(std::function<void(bool, bool, bool)> onRecallCallback, 
    std::function<void()> onCancelCallback
)   : onRecall(std::move(onRecallCallback)), onCancel(std::move(onCancelCallback))
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
    layoutToggle.setRadioGroupId(1);
    dataToggle.setRadioGroupId(1);
    fullToggle.setRadioGroupId(1);

    layoutToggle.setLookAndFeel(&boxToggleLF);
    dataToggle.setLookAndFeel(&boxToggleLF);
    fullToggle.setLookAndFeel(&boxToggleLF);

    layoutToggle.onClick = [this] { loadBtn.setEnabled(true); };
    dataToggle.onClick = [this] { loadBtn.setEnabled(true); };
    fullToggle.onClick = [this] { loadBtn.setEnabled(true); };

    addAndMakeVisible(layoutToggle);
    addAndMakeVisible(dataToggle);
    addAndMakeVisible(fullToggle);
}

void PresetLoadDialog::configLoadBtn()
{
	loadBtn.setEnabled(false);

    loadBtn.onClick = [this]
        {
            if (onRecall) 
            {
                bool isLayoutOnly = layoutToggle.getToggleState();
                bool isDataOnly = dataToggle.getToggleState();
                bool isFull = fullToggle.getToggleState();

                bool doLayout = isLayoutOnly || isFull;
                bool doData = isDataOnly || isFull;
                bool doStores = isDataOnly || isFull;

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

PresetLoadDialog::~PresetLoadDialog()
{
    layoutToggle.setLookAndFeel(nullptr);
    dataToggle.setLookAndFeel(nullptr);
    fullToggle.setLookAndFeel(nullptr);
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

    layoutToggle.setBounds(area.removeFromTop(35).reduced(0, 3));
    dataToggle.setBounds(area.removeFromTop(35).reduced(0, 3));
    fullToggle.setBounds(area.removeFromTop(35).reduced(0, 3));

    auto btnArea = area.removeFromBottom(40);
    cancelBtn.setBounds(btnArea.removeFromLeft(100));
    loadBtn.setBounds(btnArea.removeFromRight(100));
}