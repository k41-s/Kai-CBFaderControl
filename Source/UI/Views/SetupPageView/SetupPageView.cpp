#include "SetupPageView.h"
#include "../../../Main/SlotIDs.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../../Utils/StateUtils/SlotStateHelpers.h"

SetupPageView::SetupPageView(KaiCBFaderControlAudioProcessor& p) : processor(p)
{
	init();
}

void SetupPageView::init()
{
	processor.apvts.state.addListener(this);
	setLookAndFeel(&customLF);
	configComponents();
	refreshControlStates();
}

void SetupPageView::configComponents()
{
	configLocalIpLabel();

	configLabelEditorPair(
		"Target Ip Address",
		targetIpLabel,
		SlotStateHelpers::getStringProp(processor.apvts.state, SlotIDs::targetIP().toString()),
		targetIpEditor
	);

	configLabelEditorPair(
		"Incoming Port",
		incomingPortLabel,
		SlotStateHelpers::getStringProp(processor.apvts.state, SlotIDs::incomingPort().toString()),
		incomingPortEditor
	);

	configLabelEditorPair(
		"Outgoing Port",
		outgoingPortLabel,
		SlotStateHelpers::getStringProp(processor.apvts.state, SlotIDs::outgoingPort().toString()),
		outgoingPortEditor
	);

	restrictPortEditors();

	bindNetworkEditorCallbacks();

	configStatusComponents();
	configGrid();
	configToggleAllBtnText();

	configNavBtn();

	configImages();
}

static juce::String getLocalIpAddress()
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

void SetupPageView::configLabelEditorPair(
	juce::String lblTxt,
	juce::Label& label,
	juce::String editorTxt,
	juce::TextEditor& editor
) {
	addAndMakeVisible(label);
	label.setText(lblTxt, juce::dontSendNotification);

	addAndMakeVisible(editor);
	editor.setText(editorTxt);
}

void SetupPageView::restrictPortEditors()
{
	incomingPortEditor.setInputRestrictions(5, "0123456789");
	outgoingPortEditor.setInputRestrictions(5, "0123456789");
}

void SetupPageView::configStatusComponents()
{
	statusLabel.setText("Connection Status:", juce::dontSendNotification);
	addAndMakeVisible(statusLabel);
	addAndMakeVisible(statusLED);
}

void SetupPageView::configGrid()
{
	gridViewport.setViewedComponent(&gridContainer, false);
	addAndMakeVisible(gridViewport);

	configGridContainer();
}

void SetupPageView::configGridContainer()
{
	for (int i = 0; i < 32; ++i) {
		SlotConfigItem* item = new SlotConfigItem(processor, i + 1);
		item->setupAttachment(processor.apvts, i + 1);
		item->onToggleChanged = [this] { refreshControlStates(); };
		slotItems.add(item);
		gridContainer.addAndMakeVisible(item);
	}
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
	refreshControlStates();
}

void SetupPageView::configNavBtn()
{
	addAndMakeVisible(navigateBtn);
	navigateBtn.onClick = [this]()
		{
			if (onNavigateToPerformance)
				onNavigateToPerformance();
		};
}

void SetupPageView::configImages()
{
	addAndMakeVisible(logo);
	addAndMakeVisible(xPatchImg);
}

void SetupPageView::saveNetworkSettings()
{
	SlotStateHelpers::setStringProp(processor.apvts.state, SlotIDs::targetIP().toString(), targetIpEditor.getText());
	SlotStateHelpers::setIntProp(processor.apvts.state, SlotIDs::incomingPort().toString(), incomingPortEditor.getText().getIntValue());
	SlotStateHelpers::setIntProp(processor.apvts.state, SlotIDs::outgoingPort().toString(), outgoingPortEditor.getText().getIntValue());

	DBG("Network Settings Saved: " << targetIpEditor.getText());
}

void SetupPageView::bindNetworkEditorCallbacks()
{
	targetIpEditor.onReturnKey = [this] { saveNetworkSettings(); };
	incomingPortEditor.onReturnKey = [this] { saveNetworkSettings(); };
	outgoingPortEditor.onReturnKey = [this] { saveNetworkSettings(); };

	targetIpEditor.onFocusLost = [this] { saveNetworkSettings(); };
	incomingPortEditor.onFocusLost = [this] { saveNetworkSettings(); };
	outgoingPortEditor.onFocusLost = [this] { saveNetworkSettings(); };
}

SetupPageView::~SetupPageView()
{
	processor.apvts.state.removeListener(this);
	setLookAndFeel(nullptr);
}

void SetupPageView::paint(juce::Graphics& g)
{
	g.setColour(MyColours::cbBlue);
	g.fillRoundedRectangle(footerArea.toFloat(), 5.0f);
}

void SetupPageView::resized()
{
	auto area = getLocalBounds().reduced(10);

	auto leftPanel = area.removeFromLeft(200);
	setupLeftPanel(leftPanel);

	area.removeFromLeft(20);

	setupGrid(area);
}

void SetupPageView::setupLeftPanel(juce::Rectangle<int>& area)
{
	placeIpComponents(area);
	placePortComponents(area);
	placeToggleAllBtn(area);
	placeStatusComponents(area);
	placeNavigateBtn(area);
	placeImages(area);
}

void SetupPageView::placeIpComponents(juce::Rectangle<int>& area)
{
	localIpLabel.setBounds(area.removeFromTop(30));
	setLabelEditorPairBounds(area, targetIpLabel, targetIpEditor);
}

void SetupPageView::placePortComponents(juce::Rectangle<int>& area)
{
	setLabelEditorPairBounds(area, incomingPortLabel, incomingPortEditor);
	setLabelEditorPairBounds(area, outgoingPortLabel, outgoingPortEditor);
}

void SetupPageView::setLabelEditorPairBounds(
	juce::Rectangle<int>& area,
	juce::Label& label,
	juce::TextEditor& editor)
{
	area.removeFromTop(10);
	label.setBounds(area.removeFromTop(30));
	area.removeFromTop(5);
	editor.setBounds(area.removeFromTop(40));
}

void SetupPageView::placeToggleAllBtn(juce::Rectangle<int>& area)
{
	area.removeFromTop(20);
	toggleAllButton.setBounds(area.removeFromTop(30).reduced(2));
}

void SetupPageView::placeStatusComponents(juce::Rectangle<int>& area)
{
	area.removeFromTop(10);
	setStatusBounds(area);
}

void SetupPageView::setStatusBounds(juce::Rectangle<int>& area)
{
	auto statusRow = area.removeFromTop(30);
	statusLabel.setBounds(statusRow.removeFromLeft(100));
	statusLED.setBounds(statusRow.removeFromLeft(30).reduced(5));
}

void SetupPageView::placeNavigateBtn(juce::Rectangle<int>& area)
{
	area.removeFromTop(20);
	navigateBtn.setBounds(area.removeFromTop(40).reduced(2));
}

void SetupPageView::placeImages(juce::Rectangle<int>& area)
{
	footerArea = area.removeFromBottom(50).reduced(5);
	auto areaToUse = footerArea;
	placeCBLogo(areaToUse);
	placeXPatchImg(areaToUse);
}

void SetupPageView::placeCBLogo(juce::Rectangle<int>& areaToUse)
{
	auto logoArea = areaToUse.removeFromLeft(75);
	logo.setBounds(logoArea.withSizeKeepingCentre(50, logoArea.getHeight()).reduced(5));
}

void SetupPageView::placeXPatchImg(juce::Rectangle<int>& areaToUse)
{
	auto xPatchArea = areaToUse.removeFromRight(90);
	xPatchImg.setBounds(xPatchArea.withSizeKeepingCentre(90, xPatchArea.getHeight()).reduced(5));
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

void SetupPageView::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
	if (property == SlotIDs::targetIP())
		targetIpEditor.setText(tree[property], juce::dontSendNotification);
	if (property == SlotIDs::incomingPort())
		incomingPortEditor.setText(tree[property], juce::dontSendNotification);
	if (property == SlotIDs::outgoingPort())
		outgoingPortEditor.setText(tree[property], juce::dontSendNotification);
}

void SetupPageView::refreshControlStates()
{
	bool allAreActive = true;
	bool atLeastOneActive = false;

	if (slotItems.isEmpty())
	{
		handleEmptySlots();
		return;
	}

	checkSlotActivationStates(allAreActive, atLeastOneActive);
	updateToggleAllButton(allAreActive);
	updateNavigateBtn(atLeastOneActive);
}

void SetupPageView::handleEmptySlots()
{
	toggleAllButton.setButtonText(SELECT_ALL);
	navigateBtn.setEnabled(false);
}

void SetupPageView::checkSlotActivationStates(bool& allAreActive, bool& atLeastOneActive)
{
	for (auto* item : slotItems)
	{
		if (item != nullptr)
		{
			if (!item->isActive())
				allAreActive = false;
			else
				atLeastOneActive = true;
		}
	}
}

void SetupPageView::updateToggleAllButton(bool allAreActive)
{
	toggleAllButton.setButtonText(allAreActive ? DESELECT_ALL : SELECT_ALL);
	toggleAllButton.setToggleState(allAreActive, juce::dontSendNotification);
}

void SetupPageView::updateNavigateBtn(bool atLeastOneActive)
{
	navigateBtn.setEnabled(atLeastOneActive);
	navigateBtn.setTooltip(atLeastOneActive ? "" : "Select at least one fader to enter Performance Mode");
}

void SetupPageView::setAllSlotsActive(bool shouldBeActive)
{
	for (auto* item : slotItems)
	{
		if (item != nullptr)
			item->setToggleState(shouldBeActive, true);
	}
	refreshControlStates();
}