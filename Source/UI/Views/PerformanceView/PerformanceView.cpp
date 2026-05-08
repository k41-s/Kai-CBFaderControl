#include "PerformanceView.h"
#include "../../../Main/SlotIDs.h"
#include "../../Components/UIConstants.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../../Utils/Enums/ContextMenuId.h"
#include "../../../Utils/StateUtils/SlotStateHelpers.h"

PerformanceView::PerformanceView(KaiCBFaderControlAudioProcessor& p)
	:processor(p)
{
	init();
}

void PerformanceView::init()
{
	setLookAndFeel(&performanceLF);
	selectedItems.addChangeListener(this);
	processor.globalSlotRegistry->addChangeListener(this);
	configComponents();
	registerListeners();
	addAndMakeVisible(lasso);
	triggerAsyncUpdate();
}

void PerformanceView::configComponents()
{
	createFaderSlots();
	createVcaFaderSlots();
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

void PerformanceView::createVcaFaderSlots()
{
	for (int i = 0; i < 8; ++i)
	{
		auto* vca = vcaSlots.add(new VcaSlotItem(processor, i + 1));
		addAndMakeVisible(vca);
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
	addRegularSlotListeners();
	addVcaListeners();
	processor.apvts.state.addListener(this);
}

void PerformanceView::addRegularSlotListeners()
{
	for (int i = 1; i <= 32; ++i)
	{
		processor.apvts.addParameterListener(SlotIDs::isActive(i), this);
	}
}

void PerformanceView::addVcaListeners()
{
	for (int i = 1; i <= 8; ++i) 
	{
		processor.apvts.addParameterListener(SlotIDs::isVcaExpanded(i), this);
		processor.apvts.addParameterListener(SlotIDs::vcaEnabled(i), this);
	}
}

PerformanceView::~PerformanceView()
{
	deregisterListeners();
	selectedItems.removeChangeListener(this);
	setLookAndFeel(nullptr);
}

void PerformanceView::deregisterListeners()
{
	removeRegularSlotListeners();
	removeVcaListeners();
	processor.apvts.state.removeListener(this);
	processor.globalSlotRegistry->removeChangeListener(this);
}

void PerformanceView::removeRegularSlotListeners()
{
	for (int i = 1; i <= 32; ++i)
	{
		processor.apvts.removeParameterListener(SlotIDs::isActive(i), this);
	}
}

void PerformanceView::removeVcaListeners()
{
	for (int i = 1; i <= 8; ++i)
	{
		processor.apvts.removeParameterListener(SlotIDs::isVcaExpanded(i), this);
		processor.apvts.removeParameterListener(SlotIDs::vcaEnabled(i), this);
	}
}

void PerformanceView::parameterChanged(const juce::String& parameterID, float newValue)
{
	triggerAsyncUpdate();
}

void PerformanceView::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
	if (property.toString().startsWith("isStereo") 
		|| property.toString().startsWith("vcaId") 
		|| property.toString().startsWith("groupId")
	) {
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
	if (source == &selectedItems)
	{
		for (auto* slot : slots) 
		{
			slot->setSelected(selectedItems.isSelected(slot->getIndex()));
		}
	}
	else if (source == &processor.globalSlotRegistry.get())
		triggerAsyncUpdate();
}

void PerformanceView::findLassoItemsInArea(juce::Array<int>& itemsFound, const juce::Rectangle<int>& area)
{
	for (auto* slot : slots)
	{
		if (slot->isVisible() && slot->getBounds().intersects(area))
			itemsFound.add(slot->getIndex());
	}
}

juce::SelectedItemSet<int>& PerformanceView::getLassoSelection()
{
	return selectedItems;
}

void PerformanceView::mouseDown(const juce::MouseEvent& e)
{
	if (handleIsPopupMenuEvent(e)) return;

	if (!e.mods.isShiftDown() && !e.mods.isCommandDown() && !e.mods.isCtrlDown())
		selectedItems.deselectAll();

	lasso.beginLasso(e, this);
}

bool PerformanceView::handleIsPopupMenuEvent(const juce::MouseEvent& e)
{
	if (e.mods.isPopupMenu())
	{
		if (selectedItems.getNumSelected() > 0)
			showContextMenu();

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
	if (e.mods.isPopupMenu()) 
	{
		if (!selectedItems.isSelected(slot->getIndex()))
		{
			selectedItems.deselectAll();
			selectedItems.addToSelection(slot->getIndex());
		}
		showContextMenu();
		return;
	}

	if (e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isShiftDown())
	{
		if (selectedItems.isSelected(slot->getIndex()))
			selectedItems.deselect(slot->getIndex());
		else
			selectedItems.addToSelection(slot->getIndex());
	}
	else 
	{
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

void PerformanceView::addMenuItems(const juce::Array<int>& selectedArr, juce::PopupMenu& menu)
{
	juce::Array<int> activeSlots;
	juce::Array<int> readOnlySlots;

	sortSelectedSlots(selectedArr, activeSlots, readOnlySlots);

	if (!readOnlySlots.isEmpty())
		addClaimSlotMenuItem(readOnlySlots, menu);

	if (!activeSlots.isEmpty())
		addStandardMenuOptions(readOnlySlots, menu, activeSlots);
}

void PerformanceView::sortSelectedSlots(const juce::Array<int>& selectedArr, juce::Array<int>& activeSlots, juce::Array<int>& readOnlySlots)
{
	for (int idx : selectedArr)
	{
		if (isSlotFullAccess(idx)) 
			activeSlots.add(idx);
		else 
			readOnlySlots.add(idx);
	}
}

void PerformanceView::addClaimSlotMenuItem(juce::Array<int>& readOnlySlots, juce::PopupMenu& menu)
{
	juce::String text = readOnlySlots.size() == 1
		? "Claim Slot " + juce::String(readOnlySlots[0])
		: "Claim Selected Slots";

	menu.addItem(ClaimSlot, text);
}

void PerformanceView::addStandardMenuOptions(juce::Array<int>& readOnlySlots, juce::PopupMenu& menu, juce::Array<int>& activeSlots)
{
	if (!readOnlySlots.isEmpty())
		menu.addSeparator();

	addStereoMenuItems(activeSlots, menu);
	addGroupMenu(activeSlots, menu);

	if (activeSlots.size() == 1)
		addSingleSlotGroupOptions(activeSlots, menu);
}

void PerformanceView::addStereoMenuItems(const juce::Array<int>& selectedArr, juce::PopupMenu& menu)
{
	if (selectedArr.size() == 2)
	{
		if (!SlotStateHelpers::isStereoLinked(processor.apvts.state, selectedArr[0]) &&
			!SlotStateHelpers::isStereoLinked(processor.apvts.state, selectedArr[1])
		) {
			menu.addItem(1, "Stereo Link Pairs");
		}
	}
	else if (selectedArr.size() == 1)
	{
		if (SlotStateHelpers::isStereoLinked(processor.apvts.state, selectedArr[0]))
			menu.addItem(2, "Unlink Stereo Pair");
	}
}

void PerformanceView::addGroupMenu(const juce::Array<int>& selectedArr, juce::PopupMenu& menu)
{
	menu.addSeparator();
	juce::PopupMenu groupMenu;
	setupGroupMenu(selectedArr, groupMenu);
	menu.addSubMenu("Grouping", groupMenu);
}

void PerformanceView::setupGroupMenu(const juce::Array<int>& selectedArr, juce::PopupMenu& groupMenu) const
{
	for (int i = 1; i <= 8; ++i)
		groupMenu.addItem(AssignGroupBase + i, "Assign to Group " + juce::String(i));

	groupMenu.addSeparator();

	int slotIdx = selectedArr[0];
	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
	
	if (grpId > 0)
		groupMenu.addItem(RemoveGroup, "Remove from Group");
}

void PerformanceView::addSingleSlotGroupOptions(const juce::Array<int>& selectedArr, juce::PopupMenu& menu)
{
	int slotIdx = selectedArr[0];
	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
	int role = SlotStateHelpers::getIntProp(processor.apvts.state, SlotIDs::groupRole(slotIdx));

	if (grpId > 0)
	{
		menu.addSeparator();
		addGroupMemberItems(role, menu);
		addVcaMenuItem(menu, grpId);
		setupAndAddColourMenu(menu, grpId);
	}
}

void PerformanceView::addGroupMemberItems(int role, juce::PopupMenu& menu)
{
	if (role == 0)
		menu.addItem(PromoteLeader, "Promote to Group Leader");
	else if (role == 1)
		menu.addItem(DemoteMember, "Demote to Standard Member");
}

void PerformanceView::addVcaMenuItem(juce::PopupMenu& menu, int grpId)
{
	menu.addSeparator();
	bool vcaEnabled = *processor.apvts.getRawParameterValue(SlotIDs::vcaEnabled(grpId)) > 0.5f;
	menu.addItem(ToggleVCA, vcaEnabled ? "Disable VCA Master" : "Enable VCA Master", true, false);
}

void PerformanceView::setupAndAddColourMenu(juce::PopupMenu& menu, int grpId)
{
	menu.addSeparator();
	juce::PopupMenu colourMenu;
	setupColourMenu(grpId, colourMenu);
	menu.addSubMenu("Group Colour", colourMenu);
}

void PerformanceView::setupColourMenu(int grpId, juce::PopupMenu& colourMenu) const
{
	int currentColourIdx = SlotStateHelpers::getGroupColour(processor.apvts.state, grpId);

	for (int i = 0; i < GroupColours::numColours; ++i)
		colourMenu.addItem(AssignColourBase + i, GroupColours::names[i], true, currentColourIdx == i);
}

void PerformanceView::showPopupMenuIfNotEmpty(juce::PopupMenu& menu, const juce::Array<int>& selectedArr)
{
	if (menu.getNumItems() > 0) {
		menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(this), [this, selectedArr](int result) 
		{
			handlePopupMenuResult(result, selectedArr);
		});
	}
}

void PerformanceView::handlePopupMenuResult(int result, const juce::Array<int>& selectedArr)
{
	if (result == 0 || selectedArr.isEmpty()) return;

	if (result == ClaimSlot) 
	{
		handleClaimSlot(selectedArr);
		return;
	}

	juce::Array<int> activeSlots;
	fillActiveSlots(selectedArr, activeSlots);

	if (activeSlots.isEmpty()) return;

	if (result > AssignGroupBase && result <= AssignGroupBase + GroupColours::numColours) 
	{
		handleGroupAssignment(result, activeSlots);
		return;
	}

	if (result >= AssignColourBase && result < AssignColourBase + GroupColours::numColours) 
	{
		handleColourAssignment(activeSlots, result);
		return;
	}

	switch (result) {
		case StereoLink:
			if (activeSlots.size() == 2) doStereoLink(activeSlots[0], activeSlots[1]);
			break;

		case StereoUnlink:
			if (activeSlots.size() == 1) doStereoUnlink(activeSlots[0]);
			break;

		case RemoveGroup:
			for (int idx : activeSlots) setSlotStandardGroup(idx, 0, 0);
			break;

		case PromoteLeader:
			if (activeSlots.size() == 1) promoteToGroupLeader(activeSlots[0]);
			break;

		case DemoteMember:
			if (activeSlots.size() == 1) demoteToStandardMember(activeSlots[0]);
			break;

		case ToggleVCA:
			if (activeSlots.size() == 1) toggleVcaMaster(activeSlots[0]);
			break;

		default:
			break;
	}
}

void PerformanceView::handleClaimSlot(const juce::Array<int>& selectedArr)
{
	for (int idx : selectedArr)
	{
		if (!isSlotFullAccess(idx))
		{
			processor.globalSlotRegistry->claimSlot(idx, processor.getInstanceId());

			if (auto* param = processor.apvts.getParameter(SlotIDs::isActive(idx)))
				param->setValueNotifyingHost(1.0f);
		}
	}
	selectedItems.deselectAll();
}

void PerformanceView::fillActiveSlots(const juce::Array<int>& selectedArr, juce::Array<int>& activeSlots)
{
	for (int idx : selectedArr)
		if (isSlotFullAccess(idx))
			activeSlots.add(idx);
}

void PerformanceView::handleGroupAssignment(int result, const juce::Array<int>& selectedArr)
{
	int groupId = result - AssignGroupBase;
	for (int idx : selectedArr)
		setSlotStandardGroup(idx, groupId, 0);
}

void PerformanceView::handleColourAssignment(const juce::Array<int>& selectedArr, int result)
{
	if (selectedArr.size() == 1)
	{
		int colourIdx = result - AssignColourBase;
		int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, selectedArr[0]);

		if (grpId > 0)
			SlotStateHelpers::setGroupColour(processor.apvts.state, grpId, colourIdx);
	}
}

void PerformanceView::doStereoLink(int slotA, int slotB)
{
	if (SlotStateHelpers::isStereoLinked(processor.apvts.state, slotA) || 
		SlotStateHelpers::isStereoLinked(processor.apvts.state, slotB)
	) {
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
	SlotStateHelpers::setStereoLinked(state, mainIdx, true);
	SlotStateHelpers::setStereoMain(state, mainIdx, true);
	SlotStateHelpers::setLinkedSlotId(state, mainIdx, subIdx);
}

void PerformanceView::setSubSlotProperties(juce::ValueTree& state, int subIdx, int mainIdx)
{
	SlotStateHelpers::setStereoLinked(state, subIdx, true);
	SlotStateHelpers::setStereoMain(state, subIdx, false);
	SlotStateHelpers::setLinkedSlotId(state, subIdx, mainIdx);

	SlotStateHelpers::setGroupId(state, subIdx, 0);
	SlotStateHelpers::setGroupRole(state, subIdx, 0);
}

void PerformanceView::doStereoUnlink(int slotIdx)
{
	auto& state = processor.apvts.state;
	int linkedIdx = SlotStateHelpers::getIntProp(state, SlotIDs::linkedSlotId(slotIdx), -1);

	unlinkSlot(state, slotIdx);
	if (linkedIdx != -1) unlinkSlot(state, linkedIdx);

	selectedItems.deselectAll();
}

void PerformanceView::unlinkSlot(juce::ValueTree& state, int idx)
{
	SlotStateHelpers::removeProp(state, SlotIDs::isStereoLinked(idx));
	SlotStateHelpers::removeProp(state, SlotIDs::isStereoMain(idx));
	SlotStateHelpers::removeProp(state, SlotIDs::linkedSlotId(idx));
}

void PerformanceView::promoteToGroupLeader(int slotIdx)
{
	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);

	if (grpId == 0) return;

	demoteExistingGroupLeaders(grpId);
	setSlotStandardGroup(slotIdx, grpId, 1);
}

void PerformanceView::demoteExistingGroupLeaders(int grpId)
{
	for (int i = 1; i <= 32; ++i)
	{
		int otherGrpId = SlotStateHelpers::getGroupId(processor.apvts.state, i);
		int otherRole = SlotStateHelpers::getIntProp(processor.apvts.state, SlotIDs::groupRole(i));

		if (otherGrpId == grpId && otherRole == 1) 
		{
			setSlotStandardGroup(i, grpId, 0);
		}
	}
}

void PerformanceView::setSlotStandardGroup(int slotIdx, int groupId, int role)
{
	auto& state = processor.apvts.state;
	SlotStateHelpers::setGroupId(state, slotIdx, groupId);
	SlotStateHelpers::setGroupRole(state, slotIdx, role);
}

void PerformanceView::demoteToStandardMember(int slotIdx)
{
	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
	setSlotStandardGroup(slotIdx, grpId, 0);
}

void PerformanceView::toggleVcaMaster(int slotIdx)
{
	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
	if (auto* param = processor.apvts.getParameter(SlotIDs::vcaEnabled(grpId)))
		param->setValueNotifyingHost(param->getValue() > 0.5f ? 0.0f : 1.0f);
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

	float baselineWidth = SlotSizeValues::monoSlotMinWidth;
	for (int i = 0; i < 32; ++i) 
	{
		auto info = getSlotDisplayInfo(i);
		if (info.shouldProcess && info.isVisible && !info.isStereoMain) 
		{
			baselineWidth = slots[i]->getWidth();
			break;
		}
	}

	performanceLF.updateGlobalTypography(baselineWidth);

	for (auto* item : slots) 
	{
		if (item->isVisible()) 
		{
			item->setTargetSlotWidth(baselineWidth);
			item->updateTypography();
		}
	}

	for (auto* vcaItem : vcaSlots) 
	{
		if (vcaItem->isVisible()) 
		{
			vcaItem->setTargetSlotWidth(baselineWidth);
			vcaItem->updateTypography();
		}
	}
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
	plotRegularSlots(flexBox);
	plotVcaMasters(flexBox);
}

void PerformanceView::plotRegularSlots(juce::FlexBox& flexBox)
{
	for (int i = 0; i < 32; ++i)
	{
		auto info = getSlotDisplayInfo(i);

		if (info.mode == SlotMode::Disabled || !info.shouldProcess)
		{
			slots[i]->setVisible(false);
			continue;
		}

		slots[i]->setMode(info.mode);
		slots[i]->setVisible(info.isVisible);

		if (info.isVisible)
			addSlotIfActive(true, flexBox, slots[i], info.isStereoMain);
	}
}

void PerformanceView::hideSlotIfVcaCollapsed(int grpId, bool& shouldShow)
{
	if (grpId >= 1 && grpId <= 8)
	{
		bool vcaEnabled = *processor.apvts.getRawParameterValue(SlotIDs::vcaEnabled(grpId)) > 0.5f;
		bool isExpanded = *processor.apvts.getRawParameterValue(SlotIDs::isVcaExpanded(grpId)) > 0.5f;

		if (vcaEnabled && !isExpanded) {
			shouldShow = false;
		}
	}
}

void PerformanceView::plotVcaMasters(juce::FlexBox& flexBox)
{
	for (int g = 0; g < 8; ++g)
	{
		bool vcaEnabled = *processor.apvts.getRawParameterValue(SlotIDs::vcaEnabled(g + 1)) > 0.5f;

		if (vcaEnabled) {
			vcaSlots[g]->setVisible(true);
			flexBox.items.add(juce::FlexItem(*vcaSlots[g])
				.withMaxWidth(SlotSizeValues::vcaSlotMaxWidth)
				.withFlex(SlotSizeValues::vcaSlotFlexGrowFactor));
		}
		else {
			vcaSlots[g]->setVisible(false);
		}
	}
}

void PerformanceView::addSlotIfActive(bool isActive, juce::FlexBox& flexBox, PerformanceSlotItem* slot, bool isMainStereo)
{
	if (isActive) {
		float flexGrow = isMainStereo ? SlotSizeValues::stereoSlotFlexGrowFactor : SlotSizeValues::monoSlotFlexGrowFactor;
		float maxWidth = isMainStereo ? SlotSizeValues::stereoSlotMaxWidth : SlotSizeValues::monoSlotMaxWidth;

		flexBox.items.add(juce::FlexItem(*slot)
			.withMaxWidth(maxWidth)
			.withFlex(flexGrow));
	}
}

int PerformanceView::getIdealWidth()
{
	int targetWidth = 0;
	int activeCount = 0;

	calculateRegularSlotTargetWidth(targetWidth, activeCount);
	calculateVcaTargetWidth(targetWidth, activeCount);

	if (activeCount == 0)
		return WindowSizeValues::absolutePerfMinWidth;

	return juce::jlimit(
		WindowSizeValues::absolutePerfMinWidth,
		WindowSizeValues::maxWidth,
		targetWidth
	);
}

void PerformanceView::calculateRegularSlotTargetWidth(int& targetWidth, int& activeCount)
{
	for (int i = 0; i < 32; ++i)
	{
		auto info = getSlotDisplayInfo(i);

		if (info.shouldProcess && info.isVisible)
		{
			targetWidth += info.isStereoMain ? SlotSizeValues::stereoSlotTargetWidth : SlotSizeValues::monoSlotTargetWidth;
			activeCount++;
		}
	}
}

void PerformanceView::calculateVcaTargetWidth(int& targetWidth, int& activeCount)
{
	for (int g = 0; g < 8; ++g)
	{
		bool vcaEnabled = *processor.apvts.getRawParameterValue(SlotIDs::vcaEnabled(g + 1)) > 0.5f;

		if (vcaEnabled) {
			targetWidth += SlotSizeValues::vcaSlotTargetWidth;
			activeCount++;
		}
	}
}

int PerformanceView::getMinWidth()
{
	int minWidth = 0;
	int activeCount = 0;

	calcRegularSlotMinWidth(minWidth, activeCount);
	calcVcaMinWidth(minWidth, activeCount);

	if (activeCount == 0)
		return WindowSizeValues::absolutePerfMinWidth;

	return juce::jmax(WindowSizeValues::absolutePerfMinWidth, minWidth);
}

void PerformanceView::calcRegularSlotMinWidth(int& minWidth, int& activeCount)
{
	for (int i = 0; i < 32; ++i) {
		auto info = getSlotDisplayInfo(i);

		if (info.shouldProcess && info.isVisible) {
			minWidth += info.isStereoMain ? SlotSizeValues::stereoSlotMinWidth : SlotSizeValues::monoSlotMinWidth;
			activeCount++;
		}
	}
}

void PerformanceView::calcVcaMinWidth(int& minWidth, int& activeCount)
{
	for (int g = 0; g < 8; ++g) {
		bool vcaEnabled = *processor.apvts.getRawParameterValue(SlotIDs::vcaEnabled(g + 1)) > 0.5f;
		if (vcaEnabled) {
			minWidth += SlotSizeValues::vcaSlotMinWidth;
			activeCount++;
		}
	}
}

PerformanceView::SlotDisplayInfo PerformanceView::getSlotDisplayInfo(int i)
{
	SlotDisplayInfo info;

	bool isLocallyActive = *processor.isActiveParams[i] > 0.5f;
	info.mode = processor.globalSlotRegistry->getSlotMode(i + 1, processor.getInstanceId(), isLocallyActive);

	if (info.mode == SlotMode::Disabled)
		return info;

	bool isLinked = SlotStateHelpers::isStereoLinked(processor.apvts.state, i + 1);
	info.isStereoMain = SlotStateHelpers::isStereoMain(processor.apvts.state, i + 1);

	if (isLinked && !info.isStereoMain)
	{
		int linkedIdx = SlotStateHelpers::getLinkedSlotId(processor.apvts.state, i + 1);
		if (linkedIdx != -1 && *processor.isActiveParams[linkedIdx - 1] < 0.5f)
		{
			// Orphaned sub-slot, treat it as a standard mono slot
		}
		else
		{
			return info;
		}
	}

	info.shouldProcess = true;

	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, i + 1);
	info.isVisible = true;
	hideSlotIfVcaCollapsed(grpId, info.isVisible);

	return info;
}

bool PerformanceView::isSlotFullAccess(int slotIdx)
{
	bool isLocallyActive = *processor.isActiveParams[slotIdx - 1] > 0.5f;
	return processor.globalSlotRegistry
		->getSlotMode(slotIdx, processor.getInstanceId(), isLocallyActive) == SlotMode::FullAccess;
}
