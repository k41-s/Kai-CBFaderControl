/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "../../UI/Components/UIConstants.h"
#include "../../UI/CustomLookAndFeel/MyColours.h"
#include "../../Utils/StateUtils/SlotStateHelpers.h"

//==============================================================================
KaiCBFaderControlAudioProcessorEditor::KaiCBFaderControlAudioProcessorEditor (KaiCBFaderControlAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), setupPage(p), performanceView(p)
{
    init();
}

void KaiCBFaderControlAudioProcessorEditor::init()
{
    addScreens();
    configSetupPageLambdas();
    configPerformanceViewLambdas();
    determineInitialState();
    configResizing();
    setAppropriateSize();
}

void KaiCBFaderControlAudioProcessorEditor::addScreens()
{
    addAndMakeVisible(setupPage);
    addAndMakeVisible(performanceView);
}


void KaiCBFaderControlAudioProcessorEditor::configSetupPageLambdas()
{
    setupPage.onNavigateToPerformance = [this]()
        {
            showSetupPage = false;
            updateWindowSize(performanceView.getCurrentPreservedWidth(), getHeight());
            resized();
        };
}

void KaiCBFaderControlAudioProcessorEditor::configPerformanceViewLambdas()
{
    performanceView.onNavigateToSetup = [this]()
        {
            showSetupPage = true;
            setSize(WindowSizeValues::defaultWidth, WindowSizeValues::defaultHeight);
            resized();
        };

    performanceView.onLayoutChangeRequest = [this]()
        {
            if (!showSetupPage)
            {
                updateWindowSize(performanceView.getCurrentPreservedWidth(), getHeight());
            }
        };
}

void KaiCBFaderControlAudioProcessorEditor::determineInitialState()
{
    showSetupPage = true;
    for (int i = 0; i < PluginConstants::numSlots; ++i)
    {
        if (SlotStateHelpers::isSlotActive(audioProcessor.apvts, i + 1))
        {
            showSetupPage = false;
            break;
        }
    }
}

void KaiCBFaderControlAudioProcessorEditor::configResizing()
{
    setResizable(true, true);
}

void KaiCBFaderControlAudioProcessorEditor::setAppropriateSize()
{
    int initialWidth = showSetupPage 
        ? WindowSizeValues::defaultWidth 
        : performanceView.getCurrentPreservedWidth();
    int initialHeight = WindowSizeValues::defaultHeight;
    updateWindowSize(initialWidth, initialHeight);
}

KaiCBFaderControlAudioProcessorEditor::~KaiCBFaderControlAudioProcessorEditor()
{
}

void KaiCBFaderControlAudioProcessorEditor::userTriedToCloseWindow()
{
    // Check with the Performance View if there are any active unsaved changes
    if (performanceView.getHasUnsavedChanges())
    {
        juce::AlertWindow::showAsync(
            juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::WarningIcon)
            .withTitle("Unsaved Changes")
            .withMessage("You have unsaved changes. Are you sure you want to close?")
            .withButton("Close anyway").withButton("Cancel"),
            [this](int result) 
            {
                if (result == 1) 
                {
                    if (auto* dw = findParentComponentOfClass<juce::DocumentWindow>())
                        dw->closeButtonPressed();
                }
            }
        );
    }
    else
    {
        // Safe to close natively
        if (auto* dw = findParentComponentOfClass<juce::DocumentWindow>())
            dw->closeButtonPressed();
    }
}

//==============================================================================
void KaiCBFaderControlAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(MyColours::background);
}

void KaiCBFaderControlAudioProcessorEditor::resized()
{
    showCurrentView(showSetupPage);
}

void KaiCBFaderControlAudioProcessorEditor::showCurrentView(bool showSetupPage)
{
    setupPage.setVisible(showSetupPage);
    performanceView.setVisible(!showSetupPage);

    auto maxSize = getMaxConstrainedWindowSize();
    int trueMaxWidth = maxSize.x;
    int trueMaxHeight = maxSize.y;

    if (showSetupPage)
    {
        int safeMinWidth = juce::jmin(WindowSizeValues::setupMinWidth, trueMaxWidth);
        int safeMinHeight = juce::jmin(WindowSizeValues::setupMinHeight, trueMaxHeight);

        setResizeLimits(
            safeMinWidth,
            safeMinHeight,
            trueMaxWidth,
            trueMaxHeight);

        setupPage.setBounds(getLocalBounds());
    }
    else
    {
        int perfMinWidth = juce::jmin(performanceView.getMinWidth(), trueMaxWidth);
        int perfMinHeight = juce::jmin(WindowSizeValues::perfMinHeight, trueMaxHeight);

        setResizeLimits(perfMinWidth,
            perfMinHeight,
            trueMaxWidth,
            trueMaxHeight);

        performanceView.setBounds(getLocalBounds());
    }
}

void KaiCBFaderControlAudioProcessorEditor::updateWindowSize(int width, int height)
{
    auto maxSize = getMaxConstrainedWindowSize();
    int trueMaxWidth = maxSize.x;
    int trueMaxHeight = maxSize.y;

    int currentMinWidth = showSetupPage ? WindowSizeValues::setupMinWidth : performanceView.getMinWidth();
    int currentMinHeight = showSetupPage ? WindowSizeValues::setupMinHeight : WindowSizeValues::perfMinHeight;

    currentMinWidth = juce::jmin(currentMinWidth, trueMaxWidth);
    currentMinHeight = juce::jmin(currentMinHeight, trueMaxHeight);

    int safeWidth = juce::jlimit(currentMinWidth, trueMaxWidth, width);
    int safeHeight = juce::jlimit(currentMinHeight, trueMaxHeight, height);

    setSize(safeWidth, safeHeight);
}

juce::Point<int> KaiCBFaderControlAudioProcessorEditor::getMaxConstrainedWindowSize()
{
    auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());
    int maxScreenWidth = display ? display->userArea.getWidth() - 40 : WindowSizeValues::maxWidth;
    int maxScreenHeight = display ? display->userArea.getHeight() - 40 : WindowSizeValues::maxHeight;

    int trueMaxWidth = juce::jmin(WindowSizeValues::maxWidth, maxScreenWidth);
    int trueMaxHeight = juce::jmin(WindowSizeValues::maxHeight, maxScreenHeight);

    return { trueMaxWidth, trueMaxHeight };
}
