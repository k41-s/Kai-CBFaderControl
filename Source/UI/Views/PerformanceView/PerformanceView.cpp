#include "PerformanceView.h"
#include "../../../Main/SlotIDs.h"
#include "../../Components/UIConstants.h"
#include "../../CustomLookAndFeel/MyColours.h"

PerformanceView::PerformanceView(KaiCBFaderControlAudioProcessor& p)
	:processor(p)
{
	init();
}

void PerformanceView::init()
{
	setLookAndFeel(&performanceLF);
	selectedItems.addChangeListener(this);
	configComponents();
	registerListeners();
	addAndMakeVisible(lasso);
	triggerAsyncUpdate();
}

void PerformanceView::configComponents()
{
	createFaderSlots();
	configSetupButton();
	configImages();
}

void PerformanceView::createFaderSlots()
{
	for (int i = 0; i < 32; ++i)
	{
		auto* slot = slots.add(new PerformanceSlotItem(processor, i + 1));
		setSlotMouseEvents(slot);
		addAndMakeVisible(slot);
	}
}

void PerformanceView::setSlotMouseEvents(PerformanceSlotItem* slot)
{
	slot->onBackgroundMouseDown = [this](const juce::MouseEvent& e, PerformanceSlotItem* s) { handleSlotMouseDown(e, s); };
	slot->onBackgroundMouseDrag = [this](const juce::MouseEvent& e, PerformanceSlotItem* s) { handleSlotMouseDrag(e, s); };
	slot->onBackgroundMouseUp = [this](const juce::MouseEvent& e, PerformanceSlotItem* s) { handleSlotMouseUp(e, s); };
}

void PerformanceView::configSetupButton()
{
	addAndMakeVisible(setupButton);
	setupButton.onClick = [this]()
		{
			if (onNavigateToSetup)
				onNavigateToSetup();
		};
}

void PerformanceView::configImages()
{
	addAndMakeVisible(cbLogo);
	addAndMakeVisible(xPatchImg);
}

void PerformanceView::registerListeners()
{
	for (int i = 1; i <= 32; ++i)
	{
		processor.apvts.addParameterListener(SlotIDs::isActive(i), this);
	}
	processor.apvts.state.addListener(this);
}

PerformanceView::~PerformanceView()
{
	deregisterListeners();
	selectedItems.removeChangeListener(this);
	setLookAndFeel(nullptr);
}

void PerformanceView::deregisterListeners()
{
	for (int i = 1; i <= 32; ++i)
	{
		processor.apvts.removeParameterListener(SlotIDs::isActive(i), this);
	}
	processor.apvts.state.removeListener(this);
}

void PerformanceView::parameterChanged(const juce::String& parameterID, float newValue)
{
	triggerAsyncUpdate();
}

void PerformanceView::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
	if (property.toString().startsWith("isStereo")) {
		triggerAsyncUpdate();
	}
}

void PerformanceView::handleAsyncUpdate()
{
	resized();

	if (onLayoutChangeRequest)
		onLayoutChangeRequest();
}

void PerformanceView::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if (source == &selectedItems) {
		for (auto* slot : slots) {
			slot->setSelected(selectedItems.isSelected(slot->getIndex()));
		}
	}
}

void PerformanceView::findLassoItemsInArea(juce::Array<int>& itemsFound, const juce::Rectangle<int>& area)
{
	for (auto* slot : slots) {
		if (slot->isVisible() && slot->getBounds().intersects(area)) {
			itemsFound.add(slot->getIndex());
		}
	}
}

juce::SelectedItemSet<int>& PerformanceView::getLassoSelection()
{
	return selectedItems;
}

void PerformanceView::mouseDown(const juce::MouseEvent& e)
{
	if (handleIsPopupMenuEvent(e)) return;

	if (!e.mods.isShiftDown() && !e.mods.isCommandDown() && !e.mods.isCtrlDown()) {
		selectedItems.deselectAll();
	}

	lasso.beginLasso(e, this);
}

bool PerformanceView::handleIsPopupMenuEvent(const juce::MouseEvent& e)
{
	if (e.mods.isPopupMenu()) {
		if (selectedItems.getNumSelected() > 0) {
			showContextMenu();
		}
		return true;
	}
	return false;
}

void PerformanceView::mouseDrag(const juce::MouseEvent& e)
{
	lasso.dragLasso(e);
}

void PerformanceView::mouseUp(const juce::MouseEvent& e)
{
	lasso.endLasso();
}

void PerformanceView::handleSlotMouseDown(const juce::MouseEvent& e, PerformanceSlotItem* slot)
{
	if (e.mods.isPopupMenu()) {
		if (!selectedItems.isSelected(slot->getIndex())) {
			selectedItems.deselectAll();
			selectedItems.addToSelection(slot->getIndex());
		}
		showContextMenu();
		return;
	}

	if (e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isShiftDown()) {
		if (selectedItems.isSelected(slot->getIndex()))
			selectedItems.deselect(slot->getIndex());
		else
			selectedItems.addToSelection(slot->getIndex());
	}
	else {
		selectedItems.deselectAll();
		selectedItems.addToSelection(slot->getIndex());
	}

	lasso.beginLasso(e.getEventRelativeTo(this), this);
}

void PerformanceView::handleSlotMouseDrag(const juce::MouseEvent& e, PerformanceSlotItem* slot)
{
	lasso.dragLasso(e.getEventRelativeTo(this));
}

void PerformanceView::handleSlotMouseUp(const juce::MouseEvent& e, PerformanceSlotItem* slot)
{
	lasso.endLasso();
}

void PerformanceView::showContextMenu()
{
	juce::PopupMenu menu;
	auto& selectedArr = selectedItems.getItemArray();

	addMenuItems(selectedArr, menu);
	showPopupMenuIfNotEmpty(menu, selectedArr);
}

bool PerformanceView::isSlotLinked(int slotIdx) const
{
	return processor.apvts.state.getProperty(juce::Identifier(SlotIDs::isStereoLinked(slotIdx)), false);
}

void PerformanceView::addMenuItems(const juce::Array<int>& selectedArr, juce::PopupMenu& menu)
{
	if (selectedArr.size() == 2) {
		if (!isSlotLinked(selectedArr[0]) && !isSlotLinked(selectedArr[1])) {
			menu.addItem(1, "Stereo Link Pairs");
		}
	}
	else if (selectedArr.size() == 1) {
		if (isSlotLinked(selectedArr[0])) {
			menu.addItem(2, "Unlink Stereo Pair");
		}
	}
}

void PerformanceView::showPopupMenuIfNotEmpty(juce::PopupMenu& menu, const juce::Array<int>& selectedArr)
{
	if (menu.getNumItems() > 0) {
		menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this), [this, selectedArr](int result) {
			if (result == 1 && selectedArr.size() == 2) {
				doStereoLink(selectedArr[0], selectedArr[1]);
			}
			else if (result == 2 && selectedArr.size() == 1) {
				doStereoUnlink(selectedArr[0]);
			}
			});
	}
}

void PerformanceView::doStereoLink(int slotA, int slotB)
{
	if (isSlotLinked(slotA) || isSlotLinked(slotB))
	{
		selectedItems.deselectAll();
		return;
	}

	auto& state = processor.apvts.state;
	int mainIdx = juce::jmin(slotA, slotB);
	int subIdx = juce::jmax(slotA, slotB);

	setMainSlotProperties(state, mainIdx, subIdx);
	setSubSlotProperties(state, subIdx, mainIdx);

	selectedItems.deselectAll();
}

void PerformanceView::setMainSlotProperties(juce::ValueTree& state, int mainIdx, int subIdx)
{
	state.setProperty(juce::Identifier(SlotIDs::isStereoLinked(mainIdx)), true, nullptr);
	state.setProperty(juce::Identifier(SlotIDs::isStereoMain(mainIdx)), true, nullptr);
	state.setProperty(juce::Identifier(SlotIDs::linkedSlotId(mainIdx)), subIdx, nullptr);
}

void PerformanceView::setSubSlotProperties(juce::ValueTree& state, int subIdx, int mainIdx)
{
	state.setProperty(juce::Identifier(SlotIDs::isStereoLinked(subIdx)), true, nullptr);
	state.setProperty(juce::Identifier(SlotIDs::isStereoMain(subIdx)), false, nullptr);
	state.setProperty(juce::Identifier(SlotIDs::linkedSlotId(subIdx)), mainIdx, nullptr);
}

void PerformanceView::doStereoUnlink(int slotIdx)
{
	auto& state = processor.apvts.state;
	int linkedIdx = state.getProperty(juce::Identifier(SlotIDs::linkedSlotId(slotIdx)), -1);

	unlinkSlot(state, slotIdx);
	if (linkedIdx != -1) unlinkSlot(state, linkedIdx);

	selectedItems.deselectAll();
}

void PerformanceView::unlinkSlot(juce::ValueTree& state, int idx)
{
	state.removeProperty(juce::Identifier(SlotIDs::isStereoLinked(idx)), nullptr);
	state.removeProperty(juce::Identifier(SlotIDs::isStereoMain(idx)), nullptr);
	state.removeProperty(juce::Identifier(SlotIDs::linkedSlotId(idx)), nullptr);
}

void PerformanceView::paint(juce::Graphics& g)
{
	g.setColour(MyColours::cbBlue);
	g.fillRect(headerArea);
	g.fillRect(footerArea);
}

void PerformanceView::resized()
{
	setHeaderArea();
	setupAndFillArea();
}

void PerformanceView::setHeaderArea()
{
	auto area = getLocalBounds();
	headerArea = area.removeFromTop(area.getHeight() * 0.045f);
}

void PerformanceView::setupAndFillArea()
{
	auto area = getLocalBounds();
	setupAndFillFooter(area);
	juce::FlexBox flexBox = configFlexBox();
	checkAndAddActiveSlots(flexBox);
	flexBox.performLayout(area);
}

void PerformanceView::setupAndFillFooter(juce::Rectangle<int>& area)
{
	footerArea = area.removeFromBottom(40);
	auto areaToUse = footerArea;

	xPatchImg.setBounds(footerArea.withSizeKeepingCentre(90, footerArea.getHeight()).reduced(5));
	setupButton.setBounds(areaToUse.removeFromLeft(100).reduced(5));

	auto logoArea = areaToUse.removeFromRight(100);
	cbLogo.setBounds(logoArea.withSizeKeepingCentre(50, logoArea.getHeight()).reduced(5));
}

juce::FlexBox PerformanceView::configFlexBox()
{
	juce::FlexBox flexBox;
	flexBox.flexDirection = juce::FlexBox::Direction::row;
	flexBox.flexWrap = juce::FlexBox::Wrap::noWrap;
	flexBox.alignContent = juce::FlexBox::AlignContent::stretch;
	flexBox.justifyContent = juce::FlexBox::JustifyContent::center;
	return flexBox;
}

void PerformanceView::checkAndAddActiveSlots(juce::FlexBox& flexBox)
{
	for (int i = 0; i < slots.size(); ++i)
	{
		auto* slot = slots[i];
		bool isActive = *processor.isActiveParams[i] > 0.5f;

		bool isStereoLinked = isSlotLinked(i + 1);
		bool isStereoMain = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::isStereoMain(i + 1)), false);

		if (isStereoLinked && !isStereoMain) {
			slot->setVisible(false);
			continue;
		}

		slot->setVisible(isActive);
		addSlotIfActive(isActive, flexBox, slot);
	}
}

void PerformanceView::addSlotIfActive(bool isActive, juce::FlexBox& flexBox, PerformanceSlotItem* slot)
{
	if (isActive) {
		bool isMain = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::isStereoMain(slot->getIndex())), false);

		float flexGrow = isMain ? 1.75f : 1.0f;
		float maxWidth = isMain ? 160.0f : 120.0f;

		flexBox.items.add(juce::FlexItem(*slot)
			.withMaxWidth(maxWidth)
			.withFlex(flexGrow));
	}
}

int PerformanceView::getIdealWidth()
{
	int targetWidth = 0;
	int activeCount = 0;

	for (int i = 0; i < 32; ++i) {
		if (*processor.isActiveParams[i] > 0.5f) {
			bool isLinked = isSlotLinked(i + 1);
			bool isMain = processor.apvts.state.getProperty(juce::Identifier(SlotIDs::isStereoMain(i + 1)), false);

			if (isLinked && !isMain) continue;

			activeCount++;
			targetWidth += (isLinked && isMain) ? 100 : 60;
		}
	}

	if (activeCount == 0)
		return WindowSizeValues::minWidth;

	return juce::jlimit(
		WindowSizeValues::minWidth,
		WindowSizeValues::maxWidth,
		targetWidth
	);
}