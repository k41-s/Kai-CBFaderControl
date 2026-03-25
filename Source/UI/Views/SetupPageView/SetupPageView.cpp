#include "SetupPageView.h"
#include "../SlotConfigItem/SlotConfigItem.h"

void SetupPageView::configLocalIpLabel()
{
	localIpLabel.setText("Local IP Address Displayed Here", juce::dontSendNotification);
	addAndMakeVisible(localIpLabel);
}

void SetupPageView::configLabelEditorPair(juce::String lblTxt,
	juce::Label& label,
	juce::String editorTxt, 
	juce::TextEditor& editor) {
	label.setText(lblTxt, juce::dontSendNotification);
	addAndMakeVisible(label);

	editor.setText(editorTxt);
	addAndMakeVisible(editor);
}

void SetupPageView::configGridContainer()
{
	for (int i = 0; i < 32; ++i) {
		SlotConfigItem* item = new SlotConfigItem(i + 1);
		slotItems.add(item);
		gridContainer.addAndMakeVisible(item);
	}
}

void SetupPageView::configGrid()
{
	gridViewport.setViewedComponent(&gridContainer, false);
	addAndMakeVisible(gridViewport);

	configGridContainer();
}

void SetupPageView::configComponents()
{
	configLocalIpLabel();

	configLabelEditorPair(
		"Target Ip Address",
		targetIpLabel,
		"127.0.0.0",
		targetIpEditor);

	configLabelEditorPair(
		"Incoming Port",
		incomingPortLabel,
		"8000",
		incomingPortEditor);

	configLabelEditorPair(
		"Outgoing Port",
		outgoingPortLabel,
		"8001",
		outgoingPortEditor);

	// statuslabel stuff to go here
	configGrid();
}

SetupPageView::SetupPageView() {
	configComponents();
}

SetupPageView::~SetupPageView()
{
}

void SetupPageView::setLabelEditorPairBounds(
	juce::Rectangle<int>& area, 
	juce::Label& label, 
	juce::TextEditor& editor)
{
	label.setBounds(area.removeFromTop(30));
	area.removeFromTop(5);
	editor.setBounds(area.removeFromTop(40));
}

void SetupPageView::setupLeftPanel(juce::Rectangle<int>& area)
{
	auto leftPanel = area.removeFromLeft(200);

	localIpLabel.setBounds(leftPanel.removeFromTop(30));

	leftPanel.removeFromTop(10);

	setLabelEditorPairBounds(leftPanel, targetIpLabel, targetIpEditor);

	leftPanel.removeFromTop(10);

	setLabelEditorPairBounds(leftPanel, incomingPortLabel, incomingPortEditor);

	leftPanel.removeFromTop(10);

	setLabelEditorPairBounds(leftPanel, outgoingPortLabel, outgoingPortEditor);

	// Status label somewhere here
}

void SetupPageView::setupSlots(int numColumns, int cellWidth, int cellHeight)
{
	for (int i = 0; i < slotItems.size(); ++i)
	{
		int column = i % numColumns;
		int row = i / numColumns;

		int xPos = column * cellWidth;
		int yPos = row * cellHeight;

		if (auto* item = slotItems[i])
		{
			item->setBounds(xPos, yPos, cellWidth, cellHeight);
		}
	}
}

void SetupPageView::setupGrid(juce::Rectangle<int>& area)
{

	gridViewport.setBounds(area);

	int numColumns = 4;
	int numRows = 8;

	int cellWidth = area.getWidth() / numColumns;
	int cellHeight = 80;

	gridContainer.setBounds(0, 0, area.getWidth(), numRows * cellHeight);

	setupSlots(numColumns, cellWidth, cellHeight);
}

void SetupPageView::resized()
{
	auto area = getLocalBounds().reduced(10);

	setupLeftPanel(area);

	area.removeFromLeft(20);

	setupGrid(area);
}

void SetupPageView::paint(juce::Graphics& g)
{
}

