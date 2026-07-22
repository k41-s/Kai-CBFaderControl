#include "SetupPageView.h"
#include "../../../Main/SlotIDs.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../../Utils/StateUtils/SlotStateHelpers.h"
#include "../../Components/UIConstants.h"
#include "../../../Utils/OSCUtils/NetworkUtils.h"

SetupPageView::SetupPageView(KaiCBFaderControlAudioProcessor& p) : processor(p)
{
	init();
}

void SetupPageView::init()
{
	processor.apvts.state.addListener(this);
	setLookAndFeel(&customLF);

	selectedItems.addChangeListener(this);

	configComponents();
	refreshControlStates();

	addAndMakeVisible(lasso);
	lasso.setInterceptsMouseClicks(false, false);
}

void SetupPageView::configComponents()
{
	configLocalIpLabel();

	configLabelEditorPair(
		"Target Ip Address",
		targetIpLabel,
		SlotStateHelpers::getTargetIP(processor.apvts.state),
		targetIpEditor
	);

	configLabelEditorPair(
		"Incoming Port",
		incomingPortLabel,
		juce::String(SlotStateHelpers::getIncomingPort(processor.apvts.state)),
		incomingPortEditor
	);

	configLabelEditorPair(
		"Outgoing Port",
		outgoingPortLabel,
		juce::String(SlotStateHelpers::getOutgoingPort(processor.apvts.state)),
		outgoingPortEditor
	);

	restrictPortEditors();

	bindNetworkEditorCallbacks();

	configStatusComponents();
	configGrid();
	configToggleAllBtnText();

	configNavBtn();

	configImages();

	configAboutButton();
}

void SetupPageView::configAboutButton()
{
	addAndMakeVisible(aboutButton);

	aboutButton.setTooltip("About this plugin");

	aboutButton.onClick = [this]()
		{
			auto aboutLabel = std::make_unique<juce::Label>("About", "Made by Kai Sabijan for CB Electronics\nVersion 1.0.0");

			aboutLabel->setJustificationType(juce::Justification::centred);
			aboutLabel->setColour(juce::Label::backgroundColourId, MyColours::valueBackground);
			aboutLabel->setColour(juce::Label::textColourId, MyColours::white);

			aboutLabel->setSize(200, 60);

			juce::CallOutBox::launchAsynchronously(std::move(aboutLabel), aboutButton.getScreenBounds(), nullptr);
		};
}

void SetupPageView::configLocalIpLabel()
{
	localIpLabel.setText("Local IP: " + NetworkUtils::getLocalIpAddress(), juce::dontSendNotification);
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
	incomingPortEditor.setInputRestrictions(5, UIStringConstants::numericChars);
	outgoingPortEditor.setInputRestrictions(5, UIStringConstants::numericChars);
}

void SetupPageView::configStatusComponents()
{
	statusLED.setConnected(SlotStateHelpers::isPluginConnected(processor.apvts.state));
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
	for (int i = 1; i <= PluginConstants::numSlots; ++i) 
	{
		SlotConfigItem* item = new SlotConfigItem(processor, i);
		setupSlotItem(item, i);
		slotItems.add(item);
		gridContainer.addAndMakeVisible(item);
	}
}

void SetupPageView::setupSlotItem(SlotConfigItem* item, int i)
{
	item->setupAttachment(processor.apvts, i);
	item->onToggleChanged = [this] { refreshControlStates(); };
	setupSlotMouseEvents(item);
}

void SetupPageView::setupSlotMouseEvents(SlotConfigItem* item)
{
	item->onBackgroundMouseDown = [this](const juce::MouseEvent& e) { processLassoDown(e); };
	item->onBackgroundMouseDrag = [this](const juce::MouseEvent& e) { processLassoDrag(e); };
	item->onBackgroundMouseUp = [this, item](const juce::MouseEvent& e) { processLassoUp(e, item); };
}

void SetupPageView::processLassoDown(const juce::MouseEvent& e)
{
	if (!e.mods.isLeftButtonDown()) return;

	isDeactivatingMode = e.mods.isCommandDown();
	selectedItems.deselectAll();

	lasso.beginLasso(e.getEventRelativeTo(this), this);
}

void SetupPageView::processLassoDrag(const juce::MouseEvent& e)
{
	if (!e.mods.isLeftButtonDown()) return;
	lasso.dragLasso(e.getEventRelativeTo(this));
}

void SetupPageView::processLassoUp(const juce::MouseEvent& e, SlotConfigItem* clickedItem)
{
	if (!e.mods.isLeftButtonDown()) return;

	lasso.endLasso();

	if (clickedItem != nullptr && !e.mouseWasDraggedSinceMouseDown())
	{
		clickedItem->setToggleState(!clickedItem->isActive(), true);
	}
	else if (e.mouseWasDraggedSinceMouseDown() && selectedItems.getNumSelected() > 0)
	{
		bool targetState = !isDeactivatingMode;

		for (int slotId : selectedItems.getItemArray())
		{
			if (auto* slot = getSlotItem(slotId))
			{
				if (slot->isActive() != targetState)
				{
					slot->setToggleState(targetState, true);
				}
			}
		}
	}

	selectedItems.deselectAll();
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
			saveNetworkSettings();
			 
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
	SlotStateHelpers::setTargetIP(processor.apvts.state, targetIpEditor.getText());
	SlotStateHelpers::setIncomingPort(processor.apvts.state, incomingPortEditor.getText().getIntValue());
	SlotStateHelpers::setOutgoingPort(processor.apvts.state, outgoingPortEditor.getText().getIntValue());

	DBG("Network Settings Saved: " << targetIpEditor.getText());
}

void SetupPageView::bindNetworkEditorCallbacks()
{
	auto bindEditor = [this](juce::TextEditor& editor) {
		editor.onReturnKey = [this] { saveNetworkSettings(); };
		editor.onFocusLost = [this] { saveNetworkSettings(); };
		};

	bindEditor(targetIpEditor);
	bindEditor(incomingPortEditor);
	bindEditor(outgoingPortEditor);
}

SetupPageView::~SetupPageView()
{
	selectedItems.removeChangeListener(this);
	processor.apvts.state.removeListener(this);
	setLookAndFeel(nullptr);
}

juce::SelectedItemSet<int>& SetupPageView::getLassoSelection()
{
	return selectedItems;
}

void SetupPageView::findLassoItemsInArea(juce::Array<int>& itemsFound, const juce::Rectangle<int>& area)
{
	auto mappedArea = gridContainer.getLocalArea(this, area);

	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		if (auto* item = getSlotItem(i))
		{
			if (item->getBounds().intersects(mappedArea))
			{
				itemsFound.add(i);
			}
		}
	}
}

void SetupPageView::mouseDown(const juce::MouseEvent& e) 
{
	processLassoDown(e);
}

void SetupPageView::mouseDrag(const juce::MouseEvent& e)
{
	processLassoDrag(e);
}

void SetupPageView::mouseUp(const juce::MouseEvent& e) 
{
	processLassoUp(e, nullptr); 
}

void SetupPageView::valueTreeRedirected(juce::ValueTree& tree)
{
	targetIpEditor.setText(SlotStateHelpers::getTargetIP(processor.apvts.state), juce::dontSendNotification);

	incomingPortEditor.setText(juce::String(SlotStateHelpers::getIncomingPort(processor.apvts.state)), juce::dontSendNotification);

	outgoingPortEditor.setText(juce::String(SlotStateHelpers::getOutgoingPort(processor.apvts.state)), juce::dontSendNotification);

	statusLED.setConnected(SlotStateHelpers::isPluginConnected(processor.apvts.state));

	refreshControlStates();
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
	placeAboutButton(area);
	placeIpComponents(area);
	placePortComponents(area);
	placeToggleAllBtn(area);
	placeStatusComponents(area);
	placeNavigateBtn(area);

	placeImages(area);
}

void SetupPageView::placeAboutButton(juce::Rectangle<int>& area)
{
	int btnWidth = 50;
	int btnHeight = 24;
	auto row = area.removeFromTop(btnHeight);

	auto buttonArea = row.removeFromLeft(btnWidth).withSizeKeepingCentre(btnWidth, btnHeight);
	aboutButton.setBounds(buttonArea);
}

void SetupPageView::placeIpComponents(juce::Rectangle<int>& area)
{
	localIpLabel.setBounds(area.removeFromTop(UISizeConstants::setupLabelHeight));
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
	area.removeFromTop(UISizeConstants::setupSectionPadding);
	label.setBounds(area.removeFromTop(UISizeConstants::setupLabelHeight));
	area.removeFromTop(UISizeConstants::slotPadding);
	editor.setBounds(area.removeFromTop(UISizeConstants::setupEditorHeight));
}

void SetupPageView::placeToggleAllBtn(juce::Rectangle<int>& area)
{
	area.removeFromTop(UISizeConstants::setupSectionPadding * 2);
	toggleAllButton.setBounds(area.removeFromTop(UISizeConstants::setupLabelHeight).reduced(2));
}

void SetupPageView::placeStatusComponents(juce::Rectangle<int>& area)
{
	area.removeFromTop(UISizeConstants::setupSectionPadding);
	setStatusBounds(area);
}

void SetupPageView::setStatusBounds(juce::Rectangle<int>& area)
{
	auto statusRow = area.removeFromTop(UISizeConstants::setupSectionPadding * 3);
	statusLabel.setBounds(statusRow.removeFromLeft(100));
	statusLED.setBounds(statusRow.removeFromLeft(UISizeConstants::setupLabelHeight).reduced(5));
}

void SetupPageView::placeNavigateBtn(juce::Rectangle<int>& area)
{
	area.removeFromTop(UISizeConstants::setupSectionPadding * 2);
	navigateBtn.setBounds(area.removeFromTop(UISizeConstants::setupEditorHeight).reduced(2));
}

void SetupPageView::placeImages(juce::Rectangle<int>& area)
{
	footerArea = area.removeFromBottom(UISizeConstants::setupSectionPadding * 5).reduced(5);
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

	int numColumns = 0;
	int numRows = 0;
	calculateGridDimensions(numColumns, numRows);

	int cellWidth = area.getWidth() / numColumns;
	int cellHeight = area.getHeight() / numRows;

	gridContainer.setBounds(0, 0, area.getWidth(), numRows * cellHeight);

	setupSlots(numColumns, cellWidth, cellHeight);
}

void SetupPageView::calculateGridDimensions(int& numColumns, int& numRows) const
{
	int maxColumns = 5;
	numColumns = 4;

	for (int c = maxColumns; c >= 3; --c)
	{
		if (PluginConstants::numSlots % c == 0)
		{
			numColumns = c;
			break;
		}
	}

	numRows = std::ceil(PluginConstants::numSlots / (float)numColumns);
}

void SetupPageView::setupSlots(int numColumns, int cellWidth, int cellHeight)
{
	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		int zeroBasedIndex = i - 1;
		int column = zeroBasedIndex % numColumns;
		int row = zeroBasedIndex / numColumns;

		int xPos = column * cellWidth;
		int yPos = row * cellHeight;

		if (auto* item = getSlotItem(i))
		{
			item->setBounds(xPos, yPos, cellWidth, cellHeight);
		}
	}
}

void SetupPageView::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
	if (property == SlotIDs::targetIP())
		targetIpEditor.setText(tree[property], juce::dontSendNotification);
	else if (property == SlotIDs::incomingPort())
		incomingPortEditor.setText(tree[property], juce::dontSendNotification);
	else if (property == SlotIDs::outgoingPort())
		outgoingPortEditor.setText(tree[property], juce::dontSendNotification);
	else if (property == SlotIDs::isConnected())
		statusLED.setConnected(SlotStateHelpers::isPluginConnected(tree));
}

void SetupPageView::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if (source == &selectedItems)
	{
		for (int i = 1; i <= PluginConstants::numSlots; ++i)
		{
			if (auto* item = getSlotItem(i))
			{
				item->setSelected(selectedItems.isSelected(i));
			}
		}
	}
}

void SetupPageView::refreshControlStates()
{
	if (slotItems.isEmpty())
	{
		handleEmptySlots();
		return;
	}

	bool allAreActive = true;
	bool atLeastOneActive = false;

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