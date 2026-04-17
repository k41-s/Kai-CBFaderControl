/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "../../UI/Components/UIConstants.h"
#include "../../UI/CustomLookAndFeel/MyColours.h"

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
            updateWindowSize(performanceView.getIdealWidth(), getHeight());
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
                updateWindowSize(performanceView.getIdealWidth(), getHeight());
            }
        };
}

void KaiCBFaderControlAudioProcessorEditor::determineInitialState()
{
    showSetupPage = true;
    for (int i = 0; i < 32; ++i)
    {
        if (*audioProcessor.isActiveParams[i] > 0.5f)
        {
            showSetupPage = false;
            break;
        }
    }
}

void KaiCBFaderControlAudioProcessorEditor::configResizing()
{
    setResizable(true, true);
    setResizeLimits(WindowSizeValues::minWidth,
        WindowSizeValues::minHeight,
        WindowSizeValues::maxWidth,
        WindowSizeValues::maxHeight);
}

void KaiCBFaderControlAudioProcessorEditor::setAppropriateSize()
{
    int initialWidth = showSetupPage ? WindowSizeValues::defaultWidth : performanceView.getIdealWidth();
	int initialHeight = WindowSizeValues::defaultHeight;
    updateWindowSize(initialWidth, initialHeight);
}

KaiCBFaderControlAudioProcessorEditor::~KaiCBFaderControlAudioProcessorEditor()
{
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
    if (showSetupPage)
    {
        setupPage.setBounds(getLocalBounds());
        performanceView.setBounds(0, 0, 0, 0);
    }
    else
    {
        performanceView.setBounds(getLocalBounds());
        setupPage.setBounds(0, 0, 0, 0);
    }
}

void KaiCBFaderControlAudioProcessorEditor::updateWindowSize(int width, int height)
{
    auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());

    int maxScreenWidth = display ? display->userArea.getWidth() - 40 : WindowSizeValues::maxWidth;
    int maxScreenHeight = display ? display->userArea.getHeight() - 40 : WindowSizeValues::maxHeight;

    int trueMaxWidth = juce::jmin(WindowSizeValues::maxWidth, maxScreenWidth);
    int trueMaxHeight = juce::jmin(WindowSizeValues::maxHeight, maxScreenHeight);

    int safeWidth = juce::jlimit(WindowSizeValues::minWidth, trueMaxWidth, width);
    int safeHeight = juce::jlimit(WindowSizeValues::minHeight, trueMaxHeight, height);

    setSize(safeWidth, safeHeight);
}
