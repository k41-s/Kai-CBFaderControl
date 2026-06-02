#include "PresetLoadDialog.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../Components/UIConstants.h"

PresetLoadDialog::PresetLoadDialog(std::function<void(const RecallScope&)> onRecallCallback,
    std::function<void()> onCancelCallback
) : onRecall(std::move(onRecallCallback)), onCancel(std::move(onCancelCallback))
{
    init();
}

void PresetLoadDialog::init()
{
    configComponents();
	updateUIStates();
}

void PresetLoadDialog::configComponents()
{
    configTitleLabel();
    configMainToggles();
    configGranularToggles();
    configLoadBtn();
    configCancelBtn();
}

void PresetLoadDialog::configTitleLabel()
{
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(UISizeConstants::maxFontSize, juce::Font::bold));
    addAndMakeVisible(titleLabel);
}

void PresetLoadDialog::configMainToggles()
{
    layoutToggle.setRadioGroupId(1);
    dataToggle.setRadioGroupId(1);
    fullToggle.setRadioGroupId(1);
    customToggle.setRadioGroupId(1);

    layoutToggle.setLookAndFeel(&boxToggleLF);
    dataToggle.setLookAndFeel(&boxToggleLF);
    fullToggle.setLookAndFeel(&boxToggleLF);
    customToggle.setLookAndFeel(&boxToggleLF);

    auto radioCallback = [this] { updateUIStates(); };
    layoutToggle.onClick = radioCallback;
    dataToggle.onClick = radioCallback;
    fullToggle.onClick = radioCallback;
    customToggle.onClick = radioCallback;

    addAndMakeVisible(layoutToggle);
    addAndMakeVisible(dataToggle);
    addAndMakeVisible(fullToggle);
    addAndMakeVisible(customToggle);
}

void PresetLoadDialog::configGranularToggles()
{
    customLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(customLabel);

    auto setupToggle = [this](juce::ToggleButton& t) 
        {
            t.setLookAndFeel(&boxToggleLF);
            t.onClick = [this] { updateUIStates(); };
            addAndMakeVisible(t);
        };

    setupToggle(volToggle); setupToggle(muteToggle); setupToggle(panToggle);
    setupToggle(soloToggle); setupToggle(vcaToggle); setupToggle(nameToggle);
    setupToggle(colourToggle); setupToggle(routingToggle); setupToggle(activeToggle);
    setupToggle(storesToggle);

    selectAllBtn.onClick = [this] { setAllGranularToggles(true); };
    deselectAllBtn.onClick = [this] { setAllGranularToggles(false); };

    addAndMakeVisible(selectAllBtn);
    addAndMakeVisible(deselectAllBtn);
}

void PresetLoadDialog::updateUIStates()
{
    bool isCustom = customToggle.getToggleState();

    // Enable/Disable granular controls based on 'Custom' radio state
    volToggle.setEnabled(isCustom); muteToggle.setEnabled(isCustom);
    panToggle.setEnabled(isCustom); soloToggle.setEnabled(isCustom);
    vcaToggle.setEnabled(isCustom); nameToggle.setEnabled(isCustom);
    colourToggle.setEnabled(isCustom); routingToggle.setEnabled(isCustom);
    activeToggle.setEnabled(isCustom); storesToggle.setEnabled(isCustom);

    selectAllBtn.setEnabled(isCustom);
    deselectAllBtn.setEnabled(isCustom);

    // Dim the section visually if it's inactive
    float alpha = isCustom ? 1.0f : 0.4f;
    customLabel.setAlpha(alpha); volToggle.setAlpha(alpha); muteToggle.setAlpha(alpha);
    panToggle.setAlpha(alpha); soloToggle.setAlpha(alpha); vcaToggle.setAlpha(alpha);
    nameToggle.setAlpha(alpha); colourToggle.setAlpha(alpha); routingToggle.setAlpha(alpha);
    activeToggle.setAlpha(alpha); storesToggle.setAlpha(alpha);

    // Guard against having custom checked but all sub-boxes empty
    if (isCustom)
    {
        bool anySelected = volToggle.getToggleState() || muteToggle.getToggleState() ||
            panToggle.getToggleState() || soloToggle.getToggleState() ||
            vcaToggle.getToggleState() || nameToggle.getToggleState() ||
            colourToggle.getToggleState() || routingToggle.getToggleState() ||
            activeToggle.getToggleState() || storesToggle.getToggleState();
        loadBtn.setEnabled(anySelected);
    }
    else 
    {
        bool mainRadioSelected = layoutToggle.getToggleState() ||
            dataToggle.getToggleState() ||
            fullToggle.getToggleState();

        loadBtn.setEnabled(mainRadioSelected);
    }
}

void PresetLoadDialog::setAllGranularToggles(bool state)
{
    volToggle.setToggleState(state, juce::dontSendNotification);
    muteToggle.setToggleState(state, juce::dontSendNotification);
    panToggle.setToggleState(state, juce::dontSendNotification);
    soloToggle.setToggleState(state, juce::dontSendNotification);
    vcaToggle.setToggleState(state, juce::dontSendNotification);
    nameToggle.setToggleState(state, juce::dontSendNotification);
    colourToggle.setToggleState(state, juce::dontSendNotification);
    routingToggle.setToggleState(state, juce::dontSendNotification);
    activeToggle.setToggleState(state, juce::dontSendNotification);
    storesToggle.setToggleState(state, juce::dontSendNotification);
    updateUIStates();
}

void PresetLoadDialog::configLoadBtn()
{
    loadBtn.onClick = [this]
        {
            if (onRecall)
            {
                RecallScope scope;

                if (layoutToggle.getToggleState()) {
                    scope = { false, false, false, false, false,  // Data: off
                              true,  true,  true,  true,  false };// Layout: on
                }
                else if (dataToggle.getToggleState()) {
                    scope = { true,  true,  true,  true,  true,   // Data: on
                              false, false, false, false, true }; // Layout: off, Stores: on
                }
                else if (fullToggle.getToggleState()) {
                    scope = { true, true, true, true, true, true, true, true, true, true };
                }
                else {
                    scope.volume = volToggle.getToggleState();
                    scope.mute = muteToggle.getToggleState();
                    scope.pan = panToggle.getToggleState();
                    scope.solo = soloToggle.getToggleState();
                    scope.vca = vcaToggle.getToggleState();
                    scope.name = nameToggle.getToggleState();
                    scope.colour = colourToggle.getToggleState();
                    scope.routing = routingToggle.getToggleState();
                    scope.activeState = activeToggle.getToggleState();
                    scope.stores = storesToggle.getToggleState();
                }

                onRecall(scope);
            }
        };
    addAndMakeVisible(loadBtn);
}

void PresetLoadDialog::configCancelBtn()
{
    cancelBtn.onClick = [this]
        {
            if (onCancel) 
                onCancel();
        };
    addAndMakeVisible(cancelBtn);
}

PresetLoadDialog::~PresetLoadDialog()
{
    layoutToggle.setLookAndFeel(nullptr); dataToggle.setLookAndFeel(nullptr);
    fullToggle.setLookAndFeel(nullptr); customToggle.setLookAndFeel(nullptr);
    volToggle.setLookAndFeel(nullptr); muteToggle.setLookAndFeel(nullptr);
    panToggle.setLookAndFeel(nullptr); soloToggle.setLookAndFeel(nullptr);
    vcaToggle.setLookAndFeel(nullptr); nameToggle.setLookAndFeel(nullptr);
    colourToggle.setLookAndFeel(nullptr); routingToggle.setLookAndFeel(nullptr);
    activeToggle.setLookAndFeel(nullptr); storesToggle.setLookAndFeel(nullptr);
}

void PresetLoadDialog::paint(juce::Graphics& g)
{
    g.fillAll(MyColours::background);

    g.setColour(juce::Colours::white.withAlpha(0.2f));
    auto bounds = getLocalBounds();
    g.drawLine(20.0f, 215.0f, (float)bounds.getWidth() - 20.0f, 215.0f, 1.0f);
}

void PresetLoadDialog::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Title
    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);

    // Main 4 Toggles
    int mainToggleHeight = 30;
    layoutToggle.setBounds(area.removeFromTop(mainToggleHeight).reduced(0, 3));
    dataToggle.setBounds(area.removeFromTop(mainToggleHeight).reduced(0, 3));
    fullToggle.setBounds(area.removeFromTop(mainToggleHeight).reduced(0, 3));
    customToggle.setBounds(area.removeFromTop(mainToggleHeight).reduced(0, 3));

    area.removeFromTop(25); // Space for dividing line (drawn in paint())

    // Custom Label & Macros
    auto customHeaderArea = area.removeFromTop(30);
    customLabel.setBounds(customHeaderArea.removeFromLeft(200));

    deselectAllBtn.setBounds(customHeaderArea.removeFromRight(90));
    customHeaderArea.removeFromRight(10);
    selectAllBtn.setBounds(customHeaderArea.removeFromRight(90));

    area.removeFromTop(10);

    // Granular 2-Column Checkboxes
    auto togglesArea = area.removeFromTop(160);
    auto leftCol = togglesArea.removeFromLeft(togglesArea.getWidth() / 2).reduced(0, 5);
    auto rightCol = togglesArea.reduced(10, 5);

    int toggleHeight = 30;
    volToggle.setBounds(leftCol.removeFromTop(toggleHeight));
    muteToggle.setBounds(leftCol.removeFromTop(toggleHeight));
    panToggle.setBounds(leftCol.removeFromTop(toggleHeight));
    soloToggle.setBounds(leftCol.removeFromTop(toggleHeight));
    vcaToggle.setBounds(leftCol.removeFromTop(toggleHeight));

    nameToggle.setBounds(rightCol.removeFromTop(toggleHeight));
    colourToggle.setBounds(rightCol.removeFromTop(toggleHeight));
    routingToggle.setBounds(rightCol.removeFromTop(toggleHeight));
    activeToggle.setBounds(rightCol.removeFromTop(toggleHeight));
    storesToggle.setBounds(rightCol.removeFromTop(toggleHeight));

    // Bottom Buttons
    auto btnArea = area.removeFromBottom(40);
    cancelBtn.setBounds(btnArea.removeFromLeft(100));
    loadBtn.setBounds(btnArea.removeFromRight(100));
}