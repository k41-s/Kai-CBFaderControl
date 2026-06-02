#include "PresetLoadDialog.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../Components/UIConstants.h"

namespace DialogConsts
{
    constexpr int toggleHeight = 30;
    constexpr int sideMargin = 20;
}

PresetLoadDialog::PresetLoadDialog(std::function<void(const RecallScope&)> onRecallCallback,
    std::function<void()> onCancelCallback
) : onRecall(std::move(onRecallCallback)), onCancel(std::move(onCancelCallback))
{
    init();
}

void PresetLoadDialog::init()
{
    configTogglesLists();
    configComponents();
    updateUIStates();
}

void PresetLoadDialog::configTogglesLists()
{
    mainToggles = { &layoutToggle, &dataToggle, &fullToggle, &customToggle };

    granularToggles = { &volToggle, &muteToggle, &panToggle, &soloToggle, &vcaToggle,
        &nameToggle, &colourToggle, &routingToggle, &activeToggle, &storesToggle };
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
    auto radioCallback = [this] { updateUIStates(); };

    for (auto* toggle : mainToggles)
    {
        toggle->setRadioGroupId(1);
        toggle->setLookAndFeel(&boxToggleLF);
        toggle->onClick = radioCallback;
        addAndMakeVisible(toggle);
    }
}

void PresetLoadDialog::configGranularToggles()
{
    customLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(customLabel);

    auto toggleCallback = [this] { updateUIStates(); };

    for (auto* toggle : granularToggles)
    {
        toggle->setLookAndFeel(&boxToggleLF);
        toggle->onClick = toggleCallback;
        addAndMakeVisible(toggle);
    }

    selectAllBtn.onClick = [this] { setAllGranularToggles(true); };
    deselectAllBtn.onClick = [this] { setAllGranularToggles(false); };

    addAndMakeVisible(selectAllBtn);
    addAndMakeVisible(deselectAllBtn);
}

void PresetLoadDialog::updateUIStates()
{
    bool isCustom = customToggle.getToggleState();
    float alpha = isCustom ? 1.0f : 0.4f;

    customLabel.setAlpha(alpha);
    selectAllBtn.setEnabled(isCustom);
    deselectAllBtn.setEnabled(isCustom);

    bool anyGranularSelected = false;

    for (auto* toggle : granularToggles)
    {
        toggle->setEnabled(isCustom);
        toggle->setAlpha(alpha);

        if (toggle->getToggleState())
            anyGranularSelected = true;
    }

    if (isCustom)
    {
        loadBtn.setEnabled(anyGranularSelected);
    }
    else
    {
        bool mainRadioSelected = false;
        for (auto* toggle : mainToggles)
        {
            if (toggle->getToggleState())
            {
                mainRadioSelected = true;
                break;
            }
        }
        loadBtn.setEnabled(mainRadioSelected);
    }
}

void PresetLoadDialog::setAllGranularToggles(bool state)
{
    for (auto* toggle : granularToggles)
    {
        toggle->setToggleState(state, juce::dontSendNotification);
    }
    updateUIStates();
}

void PresetLoadDialog::configLoadBtn()
{
    loadBtn.onClick = [this]
        {
            if (onRecall)
            {
                RecallScope scope;

                if (layoutToggle.getToggleState())
                {
                    scope.volume = scope.mute = scope.pan = scope.solo = scope.vca = scope.stores = false;
                    scope.name = scope.colour = scope.routing = scope.activeState = true;
                }
                else if (dataToggle.getToggleState())
                {
                    scope.volume = scope.mute = scope.pan = scope.solo = scope.vca = scope.stores = true;
                    scope.name = scope.colour = scope.routing = scope.activeState = false;
                }
                else if (fullToggle.getToggleState())
                {
                    scope.volume = scope.mute = scope.pan = scope.solo = scope.vca = scope.stores = true;
                    scope.name = scope.colour = scope.routing = scope.activeState = true;
                }
                else 
                {
                    setScopeStatesFromGranularToggles(scope);
                }

                onRecall(scope);
            }
        };
    addAndMakeVisible(loadBtn);
}

void PresetLoadDialog::setScopeStatesFromGranularToggles(RecallScope& scope)
{
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

void PresetLoadDialog::configCancelBtn()
{
    cancelBtn.onClick = [this] { if (onCancel) onCancel(); };
    addAndMakeVisible(cancelBtn);
}

PresetLoadDialog::~PresetLoadDialog()
{
    for (auto* toggle : mainToggles)
        toggle->setLookAndFeel(nullptr);

    for (auto* toggle : granularToggles)
        toggle->setLookAndFeel(nullptr);
}

void PresetLoadDialog::paint(juce::Graphics& g)
{
    g.fillAll(MyColours::background);

    if (separatorY > 0.0f)
    {
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawLine((float)DialogConsts::sideMargin, separatorY, (float)getWidth() - (float)DialogConsts::sideMargin, separatorY, 1.0f);
    }
}

void PresetLoadDialog::resized()
{
    auto area = getLocalBounds().reduced(20);
    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);

    setupMainToggles(area);
    
    separatorY = (float)area.getY() + 12.5f;
    area.removeFromTop(25);
    
    setupCustomGranularArea(area);
    setupBottomButtons(area);
}

void PresetLoadDialog::setupMainToggles(juce::Rectangle<int>& area)
{
    int mainToggleHeight = DialogConsts::toggleHeight;
    for (auto* toggle : mainToggles)
    {
        toggle->setBounds(area.removeFromTop(mainToggleHeight).reduced(0, 3));
    }
}

void PresetLoadDialog::setupCustomGranularArea(juce::Rectangle<int>& area)
{
    setupCustomAreaHeader(area);
    area.removeFromTop(10);
    setupGranularToggles(area);
}

void PresetLoadDialog::setupCustomAreaHeader(juce::Rectangle<int>& area)
{
    auto customHeaderArea = area.removeFromTop(DialogConsts::toggleHeight);
    customLabel.setBounds(customHeaderArea.removeFromLeft(200));

    deselectAllBtn.setBounds(customHeaderArea.removeFromRight(90));
    customHeaderArea.removeFromRight(10);
    selectAllBtn.setBounds(customHeaderArea.removeFromRight(90));
}

void PresetLoadDialog::setupGranularToggles(juce::Rectangle<int>& area)
{
    auto togglesArea = area.removeFromTop(160);
    auto leftCol = togglesArea.removeFromLeft(togglesArea.getWidth() / 2).reduced(0, 5);
    auto rightCol = togglesArea.reduced(10, 5);

    int numGranular = granularToggles.size();

    for (int i = 0; i < numGranular; ++i)
    {
        if (i < numGranular / 2)
            granularToggles[i]->setBounds(leftCol.removeFromTop(DialogConsts::toggleHeight));
        else
            granularToggles[i]->setBounds(rightCol.removeFromTop(DialogConsts::toggleHeight));
    }
}

void PresetLoadDialog::setupBottomButtons(juce::Rectangle<int>& area)
{
    auto btnArea = area.removeFromBottom(40);
    cancelBtn.setBounds(btnArea.removeFromLeft(100));
    loadBtn.setBounds(btnArea.removeFromRight(100));
}
