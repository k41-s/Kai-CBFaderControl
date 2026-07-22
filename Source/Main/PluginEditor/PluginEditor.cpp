/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "../../UI/Components/UIConstants.h"
#include "../../UI/CustomLookAndFeel/MyColours.h"
#include "../../Utils/StateUtils/SlotStateHelpers.h"
#include "../../Utils/Enums/DialogConstants.h"

//==============================================================================
CBFaderControlAudioProcessorEditor::CBFaderControlAudioProcessorEditor (CBFaderControlAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), setupPage(p), performanceView(p)
{
    init();
}

void CBFaderControlAudioProcessorEditor::init()
{
	setWantsKeyboardFocus(true);
    addScreens();
    configSetupPageLambdas();
    configPerformanceViewLambdas();
    determineInitialState();
    configResizing();
    setAppropriateSize();
}

void CBFaderControlAudioProcessorEditor::addScreens()
{
    addAndMakeVisible(setupPage);
    addAndMakeVisible(performanceView);
}


void CBFaderControlAudioProcessorEditor::configSetupPageLambdas()
{
    setupPage.onNavigateToPerformance = [this]()
        {
            showSetupPage = false;
            updateWindowSize(performanceView.getCurrentPreservedWidth(), getHeight());
            resized();
        };
}

void CBFaderControlAudioProcessorEditor::configPerformanceViewLambdas()
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

void CBFaderControlAudioProcessorEditor::determineInitialState()
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

void CBFaderControlAudioProcessorEditor::configResizing()
{
    setResizable(true, true);
}

void CBFaderControlAudioProcessorEditor::setAppropriateSize()
{
    int savedWidth = audioProcessor.lastEditorWidth;
    int savedHeight = audioProcessor.lastEditorHeight;

    auto maxSize = getMaxConstrainedWindowSize();

    bool isSavedSizeValid = (savedWidth > 0 && savedHeight > 0) &&
        (savedWidth <= maxSize.x && savedHeight <= maxSize.y);

    if (isSavedSizeValid)
    {
        updateWindowSize(savedWidth, savedHeight);
    }
    else
    {
        int initialWidth = showSetupPage
            ? WindowSizeValues::defaultWidth
            : performanceView.getCurrentPreservedWidth();
        int initialHeight = WindowSizeValues::defaultHeight;

        updateWindowSize(initialWidth, initialHeight);
    }
}

CBFaderControlAudioProcessorEditor::~CBFaderControlAudioProcessorEditor()
{
}

void CBFaderControlAudioProcessorEditor::userTriedToCloseWindow()
{
    if (performanceView.getHasUnsavedChanges())
    {
        juce::AlertWindow::showAsync(
            juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::WarningIcon)
            .withTitle(DialogStrings::UnsavedTitle)
            .withMessage("You have unsaved changes. Are you sure you want to close?")
            .withButton(DialogStrings::CloseAnywayBtn)
            .withButton(DialogStrings::CancelBtn),
            [this](int result) 
            {
                if (result == DialogActions::Confirm) 
                {
                    if (auto* dw = findParentComponentOfClass<juce::DocumentWindow>())
                        dw->closeButtonPressed();
                }
            }
        );
    }
    else
    {
        if (auto* dw = findParentComponentOfClass<juce::DocumentWindow>())
            dw->closeButtonPressed();
    }
}

//==============================================================================
void CBFaderControlAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(MyColours::background);
}

void CBFaderControlAudioProcessorEditor::resized()
{
    showCurrentView(showSetupPage);

    audioProcessor.lastEditorWidth = getWidth();
    audioProcessor.lastEditorHeight = getHeight();
}

void CBFaderControlAudioProcessorEditor::showCurrentView(bool showSetupPage)
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

void CBFaderControlAudioProcessorEditor::updateWindowSize(int width, int height)
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

juce::Point<int> CBFaderControlAudioProcessorEditor::getMaxConstrainedWindowSize()
{
    auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());
    int maxScreenWidth = display ? display->userArea.getWidth() - 40 : WindowSizeValues::maxWidth;
    int maxScreenHeight = display ? display->userArea.getHeight() - 40 : WindowSizeValues::maxHeight;

    int trueMaxWidth = juce::jmin(WindowSizeValues::maxWidth, maxScreenWidth);
    int trueMaxHeight = juce::jmin(WindowSizeValues::maxHeight, maxScreenHeight);

    return { trueMaxWidth, trueMaxHeight };
}

bool CBFaderControlAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    if (showSetupPage)
        return false;

    if (key.getModifiers().isCommandDown())
    {
        int code = key.getKeyCode();
        bool isShiftDown = key.getModifiers().isShiftDown();

        if (code == 'a' || code == 'A')
        {
            performanceView.handleSelectAll();
            return true;
        }
        if (code == 'g' || code == 'G')
        {
            if (isShiftDown)
                performanceView.handleRemoveFromGroup();
            else
                performanceView.handleGroupSelected();

            return true;
        }
        if (code == 's' || code == 'S')
        {
            performanceView.handleSaveActiveStore();
            return true;
        }
        if (code == 'z' || code == 'Z')
        {
            if (isShiftDown) 
                performanceView.handleRedo();
            else 
                performanceView.handleUndo();
            return true;
        }
        if (code == 'y' || code == 'Y')
        {
            performanceView.handleRedo();
            return true;
        }
    }

    return false;
}