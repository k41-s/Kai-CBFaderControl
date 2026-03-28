#include "SetupPageView.h"

juce::String getLocalIpAddress() 
{
	auto addresses = juce::IPAddress::getAllAddresses();

	for (const auto& addr : addresses)
	{
		auto ipString = addr.toString();

		if (ipString.contains(".") && ipString != "127.0.0.1")
			return ipString;
	}

	return "127.0.0.1";
}

void SetupPageView::configLocalIpLabel()
{
	localIpLabel.setText("Local IP: " + getLocalIpAddress(), juce::dontSendNotification);
	addAndMakeVisible(localIpLabel);
}

void SetupPageView::configLabelEditorPair(juce::String lblTxt,
	juce::Label& label,
	juce::String editorTxt, 
	juce::TextEditor& editor
) {
	label.setText(lblTxt, juce::dontSendNotification);
	addAndMakeVisible(label);

	editor.setText(editorTxt);
	addAndMakeVisible(editor);
}

void SetupPageView::configGridContainer()
{
	for (int i = 0; i < 32; ++i) {
		SlotConfigItem* item = new SlotConfigItem(i + 1);
		item->onToggleChanged = [this] { updateToggleAllBtnTxt(); };
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

void SetupPageView::configStatusComponents()
{
	statusLabel.setText("Connection Status:", juce::dontSendNotification);
	addAndMakeVisible(statusLabel);
	addAndMakeVisible(statusLED);
}

void SetupPageView::configToggleAllBtnText()
{
	addAndMakeVisible(toggleAllButton);
	toggleAllButton.setClickingTogglesState(true);
	toggleAllButton.onClick = [this]
		{
			bool newState = toggleAllButton.getToggleState();
			setAllSlotsActive(newState);
		};
	updateToggleAllBtnTxt();
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

	configStatusComponents();

	configGrid();

	configToggleAllBtnText();
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

void SetupPageView::setStatusBounds(juce::Rectangle<int>& statusRow)
{
	statusLabel.setBounds(statusRow.removeFromLeft(100));
	statusLED.setBounds(statusRow.removeFromLeft(30).reduced(5));
}

void SetupPageView::setupLeftPanel(juce::Rectangle<int>& area)
{

	localIpLabel.setBounds(area.removeFromTop(30));

	area.removeFromTop(10);

	setLabelEditorPairBounds(area, targetIpLabel, targetIpEditor);

	area.removeFromTop(10);

	setLabelEditorPairBounds(area, incomingPortLabel, incomingPortEditor);

	area.removeFromTop(10);

	setLabelEditorPairBounds(area, outgoingPortLabel, outgoingPortEditor);

	area.removeFromTop(20);
	toggleAllButton.setBounds(area.removeFromTop(30).reduced(2));

	area.removeFromTop(10);

	auto statusRow = area.removeFromTop(30);
	setStatusBounds(statusRow);
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
	int cellHeight = area.getHeight() / numRows;

	gridContainer.setBounds(0, 0, area.getWidth(), numRows * cellHeight);

	setupSlots(numColumns, cellWidth, cellHeight);
}

void SetupPageView::resized()
{
	auto area = getLocalBounds().reduced(10);

	auto leftPanel = area.removeFromLeft(200);
	setupLeftPanel(leftPanel);

	area.removeFromLeft(20);

	setupGrid(area);
}

void SetupPageView::paint(juce::Graphics& g)
{
}

void SetupPageView::updateToggleAllBtnTxt()
{
	bool allAreActive = true;

	if (slotItems.size() == 0)
	{
		toggleAllButton.setButtonText(SELECT_ALL);
		return;
	}

	for (auto* item : slotItems)
	{
		if (item != nullptr && !item->isActive())
		{
			allAreActive = false;
			break;
		}
	}
	toggleAllButton.setButtonText(allAreActive ? DESELECT_ALL : SELECT_ALL);
	toggleAllButton.setToggleState(allAreActive, juce::dontSendNotification);
}

void SetupPageView::setAllSlotsActive(bool shouldBeActive)
{
	for (auto* item : slotItems)
	{
		if (item != nullptr)
			item->setToggleState(shouldBeActive, false);
	}
	updateToggleAllBtnTxt();
}