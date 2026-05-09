#include "SetupPageView.h"
#include "../../../Main/SlotIDs.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../../Utils/StateUtils/SlotStateHelpers.h"
#include "../../Components/UIConstants.h"

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
	for (int i = 1; i <= PluginConstants::numSlots; ++i) 
	{
		SlotConfigItem* item = new SlotConfigItem(processor, i);
		item->setupAttachment(processor.apvts, i);
		item->onToggleChanged = [this] { refreshControlStates(); };

		// 1. Start the lasso
		item->onBackgroundMouseDown = [this](const juce::MouseEvent& e)
		{
			if (!e.mods.isLeftButtonDown()) return;
			selectedItems.deselectAll();
			lasso.beginLasso(e.getEventRelativeTo(this), this);
		};

		// 2. Drag the lasso
		item->onBackgroundMouseDrag = [this](const juce::MouseEvent& e)
		{
			if (!e.mods.isLeftButtonDown()) return;
			lasso.dragLasso(e.getEventRelativeTo(this));
		};

		// 3. Finish and Apply
		item->onBackgroundMouseUp = [this, item](const juce::MouseEvent& e) 
		{
			if (!e.mods.isLeftButtonDown()) return;

			// ALWAYS end the lasso immediately on mouse release
			lasso.endLasso();

			// If it was just a normal click (no dragging), toggle this specific item
			if (!e.mouseWasDraggedSinceMouseDown())
			{
				item->setToggleState(!item->isActive(), true);
			}
			// If it was a drag, invert all lassoed items
			else if (selectedItems.getNumSelected() > 0)
			{
				for (int slotId : selectedItems.getItemArray())
				{
					if (auto* i = getSlotItem(slotId))
					{
						i->setToggleState(!i->isActive(), true);
					}
				}
			}

			// Clean up
			selectedItems.deselectAll();
		};

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
	SlotStateHelpers::setTargetIP(processor.apvts.state, targetIpEditor.getText());
	SlotStateHelpers::setIncomingPort(processor.apvts.state, incomingPortEditor.getText().getIntValue());
	SlotStateHelpers::setOutgoingPort(processor.apvts.state, outgoingPortEditor.getText().getIntValue());

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
	if (!e.mods.isLeftButtonDown()) return;
	selectedItems.deselectAll();
	lasso.beginLasso(e, this);
}

void SetupPageView::mouseDrag(const juce::MouseEvent& e)
{
	if (!e.mods.isLeftButtonDown()) return;
	lasso.dragLasso(e);
}

void SetupPageView::mouseUp(const juce::MouseEvent& e)
{
	if (!e.mods.isLeftButtonDown()) return;

	lasso.endLasso();

	if (e.mouseWasDraggedSinceMouseDown() && selectedItems.getNumSelected() > 0)
	{
		for (int slotId : selectedItems.getItemArray())
		{
			if (auto* slot = getSlotItem(slotId))
			{
				slot->setToggleState(!slot->isActive(), true);
			}
		}
	}
	selectedItems.deselectAll();
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