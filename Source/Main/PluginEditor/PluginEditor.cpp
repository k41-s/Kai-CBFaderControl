/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "../../UI/Components/UIConstants.h"

//==============================================================================
KaiCBFaderControlAudioProcessorEditor::KaiCBFaderControlAudioProcessorEditor (KaiCBFaderControlAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), setupPage(p), performanceView(p)
{
    addAndMakeVisible(setupPage);
	addAndMakeVisible(performanceView);

    setupPage.onNavigateToPerformance = [this]()
        {
            showSetupPage = false;
			updateWindowSize(performanceView.getIdealWidth(), getHeight());
            resized();
        };

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

    determineInitialState();

    setResizable(true, true);
    setResizeLimits(WindowSizeValues::minWidth,
        WindowSizeValues::minHeight,
        WindowSizeValues::maxWidth,
        WindowSizeValues::maxHeight);
    setAppropriateSize();
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
    g.fillAll(juce::Colour::fromRGB(36,36,36));
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
