#include "PerformanceView.h"
#include "../../../Main/SlotIDs.h"
#include "../../../Main/PresetManager/PresetConstants.h"
#include "../../Components/UIConstants.h"
#include "../../CustomLookAndFeel/MyColours.h"
#include "../../../Utils/Enums/ContextMenuId.h"
#include "../../../Utils/Enums/StoresMenuIds.h"
#include "../../../Utils/Enums/DialogConstants.h"
#include "../../../Utils/StateUtils/SlotStateHelpers.h"
#include "../../Components/PinnedStoreButton/PinnedStoreButton.h"

PerformanceView::PerformanceView(KaiCBFaderControlAudioProcessor& p)
	:processor(p)
{
	init();
}

void PerformanceView::init()
{
	setWantsKeyboardFocus(true);
	setLookAndFeel(&performanceLF);
	selectedItems.addChangeListener(this);
	processor.globalSlotRegistry->addChangeListener(this);
	configComponents();
	registerListeners();
	addAndMakeVisible(lasso);
	triggerSettling();
}

void PerformanceView::configComponents()
{
	createFaderSlots();
	createVcaFaderSlots();
	configSetupButton();
	configPresetsButton();
	configStoresButton();
	configActiveStoreLabel();
	configImages();
}

void PerformanceView::createFaderSlots()
{
	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		auto* slot = slots.add(new PerformanceSlotItem(processor, i));
		bindSelectionEvents(slot);
		slot->onBulkToggleRequest = [this](bool isMute, bool newState, PerformanceSlotItem* s) { 
			handleBulkToggle(isMute, newState, s);
		};
		addAndMakeVisible(slot);
	}
}

void PerformanceView::createVcaFaderSlots()
{
	for (int i = 1; i <= PluginConstants::numVcas; ++i)
	{
		auto* vca = vcaSlots.add(new VcaSlotItem(processor, i));
		bindSelectionEvents(vca);
		addAndMakeVisible(vca);
	}
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

void PerformanceView::configPresetsButton()
{
	presetsButton.setButtonText(PresetTags::PresetsButtonText);
	addAndMakeVisible(presetsButton);

	presetsButton.onClick = [this]() { showPresetsMenu(); };
}

void PerformanceView::showPresetsMenu()
{
	juce::PopupMenu menu;
	menu.addItem(PresetsMenuIds::LoadPreset, "Load Preset...");
	menu.addItem(PresetsMenuIds::SavePreset, "Save Preset As...");

	menu.showMenuAsync(juce::PopupMenu::Options()
		.withTargetComponent(presetsButton)
		.withParentComponent(this),
		[this](int result)
		{
			if (result == PresetsMenuIds::LoadPreset) 
				handleLoadPresetRequest();
			else if (result == PresetsMenuIds::SavePreset) 
				handleSavePresetRequest();
		});
}

void PerformanceView::handleSavePresetRequest()
{
	fileChooser = std::make_unique<juce::FileChooser>("Save Preset",
		juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
		"*.xml");

	auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
	fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
		{
			auto file = fc.getResult();
			if (file != juce::File{})
			{
				processor.savePresetToFile(file);
				hasUnsavedChanges = false;
				triggerAsyncUpdate();
			}
		});
}

void PerformanceView::handleLoadPresetRequest()
{
	if (hasUnsavedChanges)
	{
		juce::AlertWindow::showAsync(juce::MessageBoxOptions()
			.withAssociatedComponent(this)
			.withIconType(juce::MessageBoxIconType::WarningIcon)
			.withTitle(DialogStrings::UnsavedTitle)
			.withMessage("You have unsaved changes in your active mix. Loading a preset will discard them. Continue?")
			.withButton("Yes, discard")
			.withButton("No, cancel"),
			[this](int choice)
			{
				if (choice == DialogActions::Confirm)
					launchLoadPresetChooser();
			});
	}
	else
	{
		launchLoadPresetChooser();
	}
}

void PerformanceView::launchLoadPresetChooser()
{
	fileChooser = std::make_unique<juce::FileChooser>("Load Preset",
		juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
		"*.xml");

	auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
	fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
		{
			auto file = fc.getResult();
			if (file != juce::File{})
			{
				auto xmlRoot = processor.loadPresetFile(file);
				if (xmlRoot != nullptr)
				{
					showPresetLoadDialog(std::move(xmlRoot));
				}
			}
		});
}

void PerformanceView::showPresetLoadDialog(std::unique_ptr<juce::XmlElement> xmlRoot)
{
	presetToLoadXml = std::move(xmlRoot);

	auto* dialog = new PresetLoadDialog(
		[this](const RecallScope& scope) {
			if (presetToLoadXml != nullptr)
			{
				PresetHelpers::selectivelyApplyState(processor.apvts, *processor.presetManager, *presetToLoadXml, scope);

				hasUnsavedChanges = false;
				triggerSettling();

				updatePinnedButtons();

				int currentStore = SlotStateHelpers::getActiveStoreId(processor.apvts);
				updateActiveStoreLabel(currentStore);

				presetToLoadXml.reset();
			}
			if (presetDialogWindow != nullptr)
				presetDialogWindow->exitModalState(DialogActions::Confirm);
		},
		[this]() {
			presetToLoadXml.reset();
			if (presetDialogWindow != nullptr) 
				presetDialogWindow->exitModalState(DialogActions::Cancel);
		}
	);

	dialog->setLookAndFeel(&performanceLF);
	dialog->setSize(450, 500);

	juce::DialogWindow::LaunchOptions options;
	options.content.setOwned(dialog);
	options.dialogTitle = "Selective Preset Recall";
	options.dialogBackgroundColour = MyColours::background;
	options.escapeKeyTriggersCloseButton = true;
	options.useNativeTitleBar = false;
	options.resizable = false;

	presetDialogWindow = options.launchAsync();
}

void PerformanceView::configStoresButton()
{
	storesButton.setButtonText(PresetTags::StoresButtonText);
	addAndMakeVisible(storesButton);
	storesButton.onClick = [this]() { showStoresMenu(); };
}

void PerformanceView::configActiveStoreLabel()
{
	addAndMakeVisible(activeStoreLabel);
	activeStoreLabel.setJustificationType(juce::Justification::centred);
	activeStoreLabel.setFont(juce::Font(15.0f, juce::Font::bold));
	activeStoreLabel.setColour(juce::Label::textColourId, juce::Colours::white);

	int index = SlotStateHelpers::getActiveStoreId(processor.apvts);
	updateActiveStoreLabel(index);
}

void PerformanceView::updateActiveStoreLabel(int index)
{
	if (index == PresetConstants::noStore)
	{
		activeStoreLabel.setText("Unsaved", juce::dontSendNotification);
	}
	else
	{
		juce::String name = processor.presetManager->getStoreName(index);
		activeStoreLabel.setText(name, juce::dontSendNotification);
	}
}

void PerformanceView::showStoresMenu()
{
	juce::PopupMenu menu;
	int numVisible = processor.presetManager->getNumVisibleStores();

	menu.addItem(AddMore, "Add Stores", numVisible < PresetConstants::maxStores);
	menu.addItem(RemoveStores, "Reset Num Stores", numVisible > PresetConstants::defaultStores);
	menu.addSeparator();

	menu.addItem(SavePinnedAsSet, "Save Current Pinned as Set...");

	juce::StringArray setNames = processor.presetManager->getStoreSetNames();
	if (!setNames.isEmpty())
	{
		addSetsSubMenu(setNames, menu);
	}
	menu.addSeparator();

	for (int i = 1; i <= numVisible; ++i)
	{
		addStoreSubMenu(i, menu);
	}

	menu.showMenuAsync(juce::PopupMenu::Options()
		.withTargetComponent(storesButton)
		.withParentComponent(this)
		.withMaximumNumColumns(1),
		[this](int result) { handleStoresMenuResult(result); });
}

void PerformanceView::addSetsSubMenu(juce::StringArray& setNames, juce::PopupMenu& menu)
{
	juce::PopupMenu setsMenu;
	for (int i = 0; i < setNames.size(); ++i)
	{
		juce::PopupMenu singleSetMenu;
		singleSetMenu.addItem(RecallSetBase + i, "Recall (Pin these stores)");
		singleSetMenu.addItem(BaseHideSet + i, "Hide Set");
		singleSetMenu.addItem(RemoveSetBase + i, "Remove Set");
		setsMenu.addSubMenu(setNames[i], singleSetMenu);
	}
	menu.addSubMenu("Store Sets", setsMenu);
}

void PerformanceView::addStoreSubMenu(int i, juce::PopupMenu& menu)
{
	juce::PopupMenu storeSubMenu;
	populateStoreMenuOptions(i, storeSubMenu);

	juce::String name = processor.presetManager->getStoreName(i);
	menu.addSubMenu(name, storeSubMenu);
}

void PerformanceView::populateStoreMenuOptions(int i, juce::PopupMenu& storeMenu)
{
	storeMenu.addItem(BaseRecall + i, "Recall");
	storeMenu.addItem(BaseSave + i, "Save (Overwrite)");
	storeMenu.addItem(BaseRename + i, "Rename...");
	storeMenu.addSeparator();
	storeMenu.addItem(BaseClear + i, "Clear State");

	bool isPinned = processor.presetManager->isStorePinned(i);
	storeMenu.addItem(BasePin + i, isPinned ? "Unpin from Footer" : "Pin to Footer", true, isPinned);
}

void PerformanceView::showPinnedStoreMenu(int i, juce::Button* btn)
{
	juce::PopupMenu menu;

	populateStoreMenuOptions(i, menu);

	menu.showMenuAsync(juce::PopupMenu::Options()
		.withTargetComponent(btn)
		.withParentComponent(this)
		.withMaximumNumColumns(1),
		[this](int result) { handleStoresMenuResult(result); });
}

void PerformanceView::handleStoresMenuResult(int result)
{
	if (result == 0) return;

	if (result >= BaseHideSet) 
	{
		handleHideSetMenuResult(result - BaseHideSet);
	}
	else if (result >= RemoveSetBase)
	{
		handleRemoveSetMenuResult(result);
	}
	else if (result >= RecallSetBase)
	{
		handleRecallSetMenuResult(result);
	}
	else if (result == SavePinnedAsSet)
	{
		promptForStoreSetName();
	}
	else if (result == RemoveStores)
	{
		handleRemoveStoresMenuResult();
	}
	else if (result > BaseClear)
	{
		int storeIdx = result - StoresMenuIds::BaseClear;
		processor.presetManager->clearStore(storeIdx);

		updatePinnedButtons();
	}
	else if (result == AddMore)
	{
		promptForAddMoreStores();
		return;
	}
	else if (result > BaseRename)
	{
		int index = result - BaseRename;
		promptForStoreName(index);
	}
	else if (result > BasePin)
	{
		int index = result - BasePin;
		bool isPinned = processor.presetManager->isStorePinned(index);
		processor.presetManager->setStorePinned(index, !isPinned);
		triggerAsyncUpdate();
	}
	else if (result > BaseSave)
	{
		handleStoreSaveMenuResult(result);
	}
	else if (result > BaseRecall)
	{
		handleStoreRecallMenuResult(result);
	}
}

void PerformanceView::promptForStoreSetName()
{
	auto* alert = new juce::AlertWindow("Save Store Set", "Enter a name for this set:", juce::AlertWindow::NoIcon);
	alert->addTextEditor(AlertFieldIDs::setName, "", "Set Name");
	alert->addButton(DialogStrings::SaveBtn, DialogActions::Confirm, juce::KeyPress(juce::KeyPress::returnKey));
	alert->addButton(DialogStrings::CancelBtn, DialogActions::Cancel, juce::KeyPress(juce::KeyPress::escapeKey));
	alert->setLookAndFeel(&performanceLF);

	alert->enterModalState(true, juce::ModalCallbackFunction::create([this, alert](int choice) {
		if (choice == DialogActions::Confirm)
		{
			juce::String newName = alert->getTextEditorContents(AlertFieldIDs::setName);
			if (newName.isNotEmpty())
			{
				auto pinned = processor.presetManager->getPinnedStores();
				processor.presetManager->saveStoreSet(newName, pinned);
			}
		}
		}), true);
}

void PerformanceView::handleRecallSetMenuResult(int result)
{
	int index = result - RecallSetBase;
	juce::StringArray setNames = processor.presetManager->getStoreSetNames();

	if (juce::isPositiveAndBelow(index, setNames.size()))
	{
		wipeCurrentPins();
		pinStoresFromSet(setNames, index);
		triggerAsyncUpdate();
	}
}

void PerformanceView::wipeCurrentPins()
{
	for (int i = 1; i <= PresetConstants::maxStores; ++i)
	{
		if (processor.presetManager->isStorePinned(i))
			processor.presetManager->setStorePinned(i, false);
	}
}

void PerformanceView::pinStoresFromSet(juce::StringArray& setNames, int index)
{
	juce::Array<int> storesToPin = processor.presetManager->getStoresInSet(setNames[index]);
	for (int storeId : storesToPin)
	{
		processor.presetManager->setStorePinned(storeId, true);
	}
}

void PerformanceView::handleRemoveSetMenuResult(int result)
{
	int index = result - RemoveSetBase;
	juce::StringArray setNames = processor.presetManager->getStoreSetNames();

	if (juce::isPositiveAndBelow(index, setNames.size()))
	{
		processor.presetManager->removeStoreSet(setNames[index]);
	}
}

void PerformanceView::handleHideSetMenuResult(int setIndex)
{
	auto setNames = processor.presetManager->getStoreSetNames();
	if (setIndex >= 0 && setIndex < setNames.size())
	{
		juce::String setName = setNames[setIndex];
		auto storesToHide = processor.presetManager->getStoresInSet(setName);

		for (int storeId : storesToHide)
		{
			processor.presetManager->setStorePinned(storeId, false);
		}

		updatePinnedButtons();
		triggerAsyncUpdate();
	}
}

void PerformanceView::handleRemoveStoresMenuResult()
{
	auto* alert = new juce::AlertWindow("Reset Stores",
		"How many stores would you like to keep? (Stores beyond this number will be permanently cleared).",
		juce::AlertWindow::NoIcon);

	alert->setLookAndFeel(&performanceLF);
	alert->addTextEditor(AlertFieldIDs::numStores, juce::String(PresetConstants::defaultStores), "Number of stores:");

	if (auto* editor = alert->getTextEditor(AlertFieldIDs::numStores))
	{
		editor->setInputRestrictions(3, UIStringConstants::numericChars);
	}

	alert->addButton(DialogStrings::ResetBtn, DialogActions::Confirm, juce::KeyPress(juce::KeyPress::returnKey));
	alert->addButton(DialogStrings::CancelBtn, DialogActions::Cancel, juce::KeyPress(juce::KeyPress::escapeKey));

	alert->enterModalState(true, juce::ModalCallbackFunction::create([this, alert](int choice)
		{
			if (choice == DialogActions::Confirm)
			{
				int currentVisible = processor.presetManager->getNumVisibleStores();

				int targetStores = alert->getTextEditorContents(AlertFieldIDs::numStores).getIntValue();

				targetStores = juce::jlimit(1, currentVisible, targetStores);

				for (int i = currentVisible; i > targetStores; --i)
				{
					processor.presetManager->clearStore(i);
				}

				processor.presetManager->setNumVisibleStores(targetStores);

				updatePinnedButtons();
				resized();
			}
		}), true);
}

void PerformanceView::promptForStoreName(int index)
{
	auto* alert = new juce::AlertWindow("Rename Store", "Enter a new name:", juce::AlertWindow::NoIcon);
	
	alert->setLookAndFeel(&performanceLF);
	alert->addTextEditor(AlertFieldIDs::storeName, processor.presetManager->getStoreName(index), "Name");

	if (auto* editor = alert->getTextEditor(AlertFieldIDs::storeName))
		editor->setInputRestrictions(PresetConstants::maxStoreNameLength);

	alert->addButton(DialogStrings::SaveBtn, DialogActions::Confirm, juce::KeyPress(juce::KeyPress::returnKey));
	alert->addButton(DialogStrings::CancelBtn, DialogActions::Cancel, juce::KeyPress(juce::KeyPress::escapeKey));

	alert->enterModalState(true, juce::ModalCallbackFunction::create([this, alert, index](int result)
		{
			if (result == DialogActions::Confirm)
			{
				handleStoreRename(alert, index);
			}
		}), true);
}

void PerformanceView::handleStoreRename(juce::AlertWindow* alert, int index)
{
	juce::String newName = alert->getTextEditorContents("nameField");
	if (newName.isNotEmpty())
	{
		processor.presetManager->setStoreName(index, newName);
		triggerAsyncUpdate();

		if (SlotStateHelpers::getActiveStoreId(processor.apvts) == index)
		{
			updateActiveStoreLabel(index);
		}
	}
	processor.presetManager->saveStore(index, processor.apvts.copyState());
}

void PerformanceView::promptForAddMoreStores()
{
	auto* alert = new juce::AlertWindow("Add Stores",
		"Enter the new total number of stores (Max " + juce::String(PresetConstants::maxStores) + "):",
		juce::AlertWindow::NoIcon);

	alert->setLookAndFeel(&performanceLF);
	alert->addTextEditor(AlertFieldIDs::numStores, juce::String(processor.presetManager->getNumVisibleStores()), "Number");
	alert->addButton(DialogStrings::UpdateBtn, DialogActions::Confirm, juce::KeyPress(juce::KeyPress::returnKey));
	alert->addButton(DialogStrings::CancelBtn, DialogActions::Cancel, juce::KeyPress(juce::KeyPress::escapeKey));

	alert->enterModalState(true, juce::ModalCallbackFunction::create([this, alert](int result)
		{
			if (result == DialogActions::Confirm)
			{
				int newNum = alert->getTextEditorContents(AlertFieldIDs::numStores).getIntValue();
				processor.presetManager->setNumVisibleStores(newNum);
			}
		}), true);
}

void PerformanceView::handleStoreSaveMenuResult(int result)
{
	int index = result - BaseSave;

	processor.isRestoringState = true;
	SlotStateHelpers::setActiveStoreId(processor.apvts, index);
	processor.isRestoringState = false;

	processor.presetManager->saveStore(index, processor.apvts.copyState());

	hasUnsavedChanges = false;
	triggerSettling();
	updateActiveStoreLabel(index);
}

void PerformanceView::handleStoreRecallMenuResult(int result)
{
	int index = result - BaseRecall;
	int currentIndex = SlotStateHelpers::getActiveStoreId(processor.apvts);

	if (currentIndex == index)
		processor.forceRecallStore(index);
	else
		SlotStateHelpers::setActiveStoreId(processor.apvts, index);
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
	processor.apvts.addParameterListener(PresetTags::ActiveStoreParamId, this);
}

void PerformanceView::addRegularSlotListeners()
{
	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		processor.apvts.addParameterListener(SlotIDs::isActive(i), this);
	}
}

void PerformanceView::addVcaListeners()
{
	for (int i = 1; i <= PluginConstants::numVcas; ++i)
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
	processor.apvts.removeParameterListener(PresetTags::ActiveStoreParamId, this);
}

void PerformanceView::removeRegularSlotListeners()
{
	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		processor.apvts.removeParameterListener(SlotIDs::isActive(i), this);
	}
}

void PerformanceView::removeVcaListeners()
{
	for (int i = 1; i <= PluginConstants::numVcas; ++i)
	{
		processor.apvts.removeParameterListener(SlotIDs::isVcaExpanded(i), this);
		processor.apvts.removeParameterListener(SlotIDs::vcaEnabled(i), this);
	}
}

void PerformanceView::parameterChanged(const juce::String& parameterID, float newValue)
{
	if (parameterID == PresetTags::ActiveStoreParamId)
	{
		handleActiveStoreParamChanged();
	}
	else
	{
		for (int i = 1; i <= PluginConstants::numVcas; ++i)
		{
			if (parameterID == SlotIDs::isVcaExpanded(i))
			{
				if (newValue > 0.5f)
				{
					juce::MessageManager::callAsync([this, i]() 
						{
							reactivateGroupMembers(i);
						});
				}
				break;
			}
		}

		if (!processor.isRestoringState && !isSettling && !hasUnsavedChanges)
		{
			hasUnsavedChanges = true;
		}
	}
	triggerAsyncUpdate();
}

void PerformanceView::handleActiveStoreParamChanged()
{
	juce::MessageManager::callAsync([this]() 
	{
		int index = SlotStateHelpers::getActiveStoreId(processor.apvts);
		updateActiveStoreLabel(index);
	});
}

void PerformanceView::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
	if (property.toString() != PresetTags::ActiveStoreParamId &&
		!processor.isRestoringState && 
		!isSettling && !hasUnsavedChanges)
	{
		hasUnsavedChanges = true;
		triggerAsyncUpdate();
	}

	if (SlotStateHelpers::isStereoOrGroupProperty(property.toString()))
	{
		triggerAsyncUpdate();
	}
}

void PerformanceView::valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged)
{
	hasUnsavedChanges = false;
	triggerSettling();
	handleActiveStoreParamChanged();
}

void PerformanceView::handleAsyncUpdate()
{
	juce::String oldSig = lastLayoutSignature;

	resized();
	repaint();

	if (oldSig != lastLayoutSignature && onLayoutChangeRequest)
		onLayoutChangeRequest();
}

void PerformanceView::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if (source == &selectedItems)
	{
		for (auto* slot : slots)
			slot->setSelected(selectedItems.isSelected(slot->getSelectionId()));

		for (auto* vca : vcaSlots)
			vca->setSelected(selectedItems.isSelected(vca->getSelectionId()));
	}
	else if (source == &processor.globalSlotRegistry.get())
		triggerAsyncUpdate();
}

void PerformanceView::findLassoItemsInArea(juce::Array<int>& itemsFound, const juce::Rectangle<int>& area)
{
	for (auto* slot : slots)
	{
		if (slot->isVisible() && slot->getBounds().intersects(area))
			itemsFound.add(slot->getSelectionId());
	}

	for (auto* vca : vcaSlots)
	{
		if (vca->isVisible() && vca->getBounds().intersects(area))
			itemsFound.add(vca->getSelectionId());
	}
}

juce::SelectedItemSet<int>& PerformanceView::getLassoSelection()
{
	return selectedItems;
}

void PerformanceView::mouseDown(const juce::MouseEvent& e)
{
	grabKeyboardFocus();

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

void PerformanceView::timerCallback()
{
	stopTimer();
	isSettling = false;
}

void PerformanceView::handleSlotMouseDown(const juce::MouseEvent& e, BaseSlotItem* slot)
{
	grabKeyboardFocus();

	if (e.mods.isPopupMenu())
	{
		if (!selectedItems.isSelected(slot->getSelectionId()))
		{
			selectedItems.deselectAll();
			selectedItems.addToSelection(slot->getSelectionId());
		}
		showContextMenu();
		return;
	}

	if (e.mods.isCommandDown() || e.mods.isCtrlDown() || e.mods.isShiftDown())
	{
		if (selectedItems.isSelected(slot->getSelectionId()))
			selectedItems.deselect(slot->getSelectionId());
		else
			selectedItems.addToSelection(slot->getSelectionId());
	}
	else
	{
		selectedItems.deselectAll();
		selectedItems.addToSelection(slot->getSelectionId());
	}

	lasso.beginLasso(e.getEventRelativeTo(this), this);
}

void PerformanceView::handleSlotMouseDrag(const juce::MouseEvent& e, BaseSlotItem* slot)
{
	lasso.dragLasso(e.getEventRelativeTo(this));
}

void PerformanceView::handleSlotMouseUp(const juce::MouseEvent& e, BaseSlotItem* slot)
{
	lasso.endLasso();
}

void PerformanceView::bindSelectionEvents(BaseSlotItem* slot)
{
	slot->onBackgroundMouseDown = [this](const juce::MouseEvent& e, BaseSlotItem* s) { handleSlotMouseDown(e, s); };
	slot->onBackgroundMouseDrag = [this](const juce::MouseEvent& e, BaseSlotItem* s) { handleSlotMouseDrag(e, s); };
	slot->onBackgroundMouseUp = [this](const juce::MouseEvent& e, BaseSlotItem* s) { handleSlotMouseUp(e, s); };
}

void PerformanceView::handleBulkToggle(bool isMute, bool newState, PerformanceSlotItem* slotClicked)
{
	auto selectedArrCopy = selectedItems.getItemArray();
	if (selectedArrCopy.isEmpty()) 
		return;

	for (int slotId : selectedArrCopy)
	{
		if (slotId == slotClicked->getIndex())
			continue;

		if (isSlotFullAccess(slotId))
		{
			juce::String paramId = isMute ? SlotIDs::mute(slotId) : SlotIDs::solo(slotId);
			float targetVal = newState ? 1.0f : 0.0f;

			SlotStateHelpers::setParamNormalizedIfChanged(processor.apvts, paramId, targetVal);

			if (!isMute && newState)
			{
				SlotStateHelpers::setSlotMuted(processor.apvts, slotId, false);
			}
		}
	}
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
		if (idx > PluginConstants::vcaSelectionOffset || isSlotFullAccess(idx))
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

	juce::Array<int> normalSlots;
	juce::Array<int> vcaSlots;

	// Parse selected array
	for (int id : activeSlots)
	{
		if (id > PluginConstants::vcaSelectionOffset)
			vcaSlots.add(id - PluginConstants::vcaSelectionOffset);
		else
			normalSlots.add(id);
	}

	// 2. Adaptive Menu: Single VCA Selected
	if (vcaSlots.size() == 1 && normalSlots.isEmpty())
	{
		addVcaMenuItem(menu, vcaSlots[0]);
		setupAndAddColourMenu(menu, vcaSlots[0]);
	}

	// 3. Adaptive Menu: Normal Slots Only
	if (vcaSlots.isEmpty() && normalSlots.size() > 0)
	{
		addStereoMenuItems(normalSlots, menu);
		addGroupMenu(normalSlots, menu);
		addSoloSafeMenuItem(normalSlots, menu);

		if (normalSlots.size() == 1)
			addSingleSlotGroupOptions(normalSlots, menu);
	}

	// Link Masks
	if (activeSlots.size() == 2)
	{
		int slotA = activeSlots[0];
		int slotB = activeSlots[1];

		int targetA = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, slotA);
		bool isTargetAVca = SlotStateHelpers::getCustomLinkedIsVca(processor.apvts.state, slotA);

		int trueIdB = slotB > PluginConstants::vcaSelectionOffset ? slotB - PluginConstants::vcaSelectionOffset : slotB;
		bool isBVca = slotB > PluginConstants::vcaSelectionOffset;

		bool alreadyLinkedToEachOther = (targetA == trueIdB && isTargetAVca == isBVca);

		menu.addSeparator();

		if (alreadyLinkedToEachOther)
		{
			juce::PopupMenu linkMaskMenu;
			setupLinkMaskMenu(activeSlots, linkMaskMenu);
			menu.addSubMenu("Link Masks", linkMaskMenu);
			menu.addItem(CustomUnlink, "Remove Link");
		}
		else
		{
			bool allowLink = true;
			if ((isBVca && !isTargetAVca) || (!isBVca && isTargetAVca))
			{
				int normalId = isBVca ? slotA : slotB;
				int vcaId = isBVca ? trueIdB : targetA;
				if (SlotStateHelpers::getGroupId(processor.apvts.state, normalId) == vcaId) 
					allowLink = false;
			}

			if (allowLink)
				menu.addItem(CreateCustomLink, "Link Faders");
		}
	}
	else if (activeSlots.size() == 1)
	{
		int slotId = activeSlots[0];
		int linkedTarget = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, slotId);

		if (linkedTarget != 0)
		{
			menu.addSeparator();
			juce::PopupMenu linkMaskMenu;
			setupLinkMaskMenu(activeSlots, linkMaskMenu);
			menu.addSubMenu("Link Masks", linkMaskMenu);
			menu.addItem(CustomUnlink, "Remove Link");
		}
	}
}

void PerformanceView::addStereoMenuItems(const juce::Array<int>& selectedArr, juce::PopupMenu& menu) const
{
	for (int idx : selectedArr)
	{
		if (SlotStateHelpers::isXpStereo(processor.apvts.state, idx))
			return;
	}

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

void PerformanceView::addLinkMaskMenu(const juce::Array<int>& activeSlots, const juce::Array<int>& normalSlots, const juce::Array<int>& vcaSlots, juce::PopupMenu& menu) const
{
	// Safety Check 1: Normal + VCA Collision
	if (normalSlots.size() == 1 && vcaSlots.size() == 1)
	{
		int normalId = normalSlots[0];
		int vcaId = vcaSlots[0];
		if (SlotStateHelpers::getGroupId(processor.apvts.state, normalId) == vcaId)
			return; // Abort: The fader is already controlled by this VCA
	}

	// Safety Check 2: Normal + Normal (Same Group)
	if (normalSlots.size() == 2)
	{
		int group1 = SlotStateHelpers::getGroupId(processor.apvts.state, normalSlots[0]);
		int group2 = SlotStateHelpers::getGroupId(processor.apvts.state, normalSlots[1]);
		if (SlotStateHelpers::isValidGroup(group1) && group1 == group2)
			return; // Abort: They are already ganged via a VCA/Group
	}

	menu.addSeparator();
	juce::PopupMenu linkMaskMenu;
	setupLinkMaskMenu(activeSlots, linkMaskMenu);
	menu.addSubMenu("Link Masks", linkMaskMenu);
}

void PerformanceView::setupLinkMaskMenu(const juce::Array<int>& activeSlots, juce::PopupMenu& linkMaskMenu) const
{
	// Determine visual toggle ticks based on the first selected slot's state tree properties
	int slotIdx = activeSlots[0];

	bool isInverse = SlotStateHelpers::isLinkPolarityInverse(processor.apvts.state, slotIdx);
	bool linkVolume = SlotStateHelpers::isLinkMaskVolume(processor.apvts.state, slotIdx);
	bool linkMute = SlotStateHelpers::isLinkMaskMute(processor.apvts.state, slotIdx);
	bool linkSolo = SlotStateHelpers::isLinkMaskSolo(processor.apvts.state, slotIdx);

	linkMaskMenu.addItem(LinkMaskPolarityParallel, "Parallel Link (1:1)", true, !isInverse);
	linkMaskMenu.addItem(LinkMaskPolarityInverse, "Inverse Link (Mirror)", true, isInverse);
	linkMaskMenu.addSeparator();
	linkMaskMenu.addItem(LinkMaskVolume, "Link Volume", true, linkVolume);
	linkMaskMenu.addItem(LinkMaskMute, "Link Mute", true, linkMute);

	bool showSolo = true;
	bool showPan = true;
	juce::Array<int> slotsToCheck = activeSlots;

	// If only 1 slot is selected, check itself and its target
	if (slotsToCheck.size() == 1)
	{
		int targetTrueId = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, slotIdx);
		bool targetIsVca = SlotStateHelpers::getCustomLinkedIsVca(processor.apvts.state, slotIdx);
		int targetSlotId = targetIsVca 
			? targetTrueId + PluginConstants::vcaSelectionOffset 
			: targetTrueId;

		slotsToCheck.add(targetSlotId);
	}

	for (int id : slotsToCheck)
	{
		if (id > PluginConstants::vcaSelectionOffset)
		{
			showSolo = false;
			showPan = false;
			//break;
		}
		else if (!SlotStateHelpers::isStereoMain(processor.apvts.state, id) &&
			!SlotStateHelpers::isXpStereo(processor.apvts.state, id))
		{
			showPan = false;
			//break;
		}
	}

	if (showSolo)
	{
		bool linkSolo = SlotStateHelpers::isLinkMaskSolo(processor.apvts.state, slotIdx);
		linkMaskMenu.addItem(LinkMaskSolo, "Link Solo", true, linkSolo);
	}
	if (showPan)
	{
		bool linkPan = SlotStateHelpers::isLinkMaskPan(processor.apvts.state, slotIdx);
		linkMaskMenu.addItem(LinkMaskPan, "Link Pan", true, linkPan);
	}
}

void PerformanceView::handleLinkMaskResult(int result, const juce::Array<int>& activeSlots)
{
	processor.undoManager.beginNewTransaction("Update Link Masks");

	int slotA = activeSlots[0];
	int targetTrueId = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, slotA);
	if (targetTrueId == 0) return;

	bool targetIsVca = SlotStateHelpers::getCustomLinkedIsVca(processor.apvts.state, slotA);
	int slotB = targetIsVca ? targetTrueId + PluginConstants::vcaSelectionOffset : targetTrueId;

	juce::Array<int> slotsToUpdate = { slotA, slotB };
	auto& state = processor.apvts.state;

	for (int idx : slotsToUpdate)
	{
		switch (result)
		{
		case LinkMaskPolarityParallel:
			SlotStateHelpers::setLinkPolarityInverse(state, idx, false, &processor.undoManager);
			break;
		case LinkMaskPolarityInverse:
			SlotStateHelpers::setLinkPolarityInverse(state, idx, true, &processor.undoManager);
			break;
		case LinkMaskVolume:
			SlotStateHelpers::setLinkMaskVolume(state, idx, !SlotStateHelpers::isLinkMaskVolume(state, idx), &processor.undoManager);
			break;
		case LinkMaskMute:
			SlotStateHelpers::setLinkMaskMute(state, idx, !SlotStateHelpers::isLinkMaskMute(state, idx), &processor.undoManager);
			break;
		case LinkMaskSolo:
			SlotStateHelpers::setLinkMaskSolo(state, idx, !SlotStateHelpers::isLinkMaskSolo(state, idx), &processor.undoManager);
			break;
		case LinkMaskPan:
			SlotStateHelpers::setLinkMaskPan(state, idx, !SlotStateHelpers::isLinkMaskPan(state, idx), &processor.undoManager);
			break;
		default:
			break;
		}
	}

	triggerAsyncUpdate();
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
	for (int i = 1; i <= PluginConstants::numGroups; ++i)
		groupMenu.addItem(AssignGroupBase + i, "Assign to Group " + juce::String(i));

	groupMenu.addSeparator();

	int slotIdx = selectedArr[0];
	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);

	if (SlotStateHelpers::isValidGroup(grpId))
		groupMenu.addItem(RemoveGroup, "Remove from Group");
}

void PerformanceView::addSingleSlotGroupOptions(const juce::Array<int>& selectedArr, juce::PopupMenu& menu)
{
	int slotIdx = selectedArr[0];
	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
	GroupRole role = SlotStateHelpers::getGroupRole(processor.apvts.state, slotIdx);

	if (SlotStateHelpers::isValidGroup(grpId))
	{
		menu.addSeparator();
		addGroupMemberItems(role, menu);
		addVcaMenuItem(menu, grpId);
		setupAndAddColourMenu(menu, grpId);
	}
}

void PerformanceView::addGroupMemberItems(GroupRole role, juce::PopupMenu& menu)
{
	if (role == GroupRole::Member)
		menu.addItem(PromoteLeader, "Promote to Group Leader");
	else if (role == GroupRole::Leader)
		menu.addItem(DemoteMember, "Demote to Standard Member");
}

void PerformanceView::addVcaMenuItem(juce::PopupMenu& menu, int grpId) const
{
	menu.addSeparator();
	bool vcaEnabled = SlotStateHelpers::isVcaEnabled(processor.apvts, grpId);
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
	if (menu.getNumItems() > 0) 
	{
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

	if (result >= LinkMaskBase && result <= LinkMaskPan) {
		handleLinkMaskResult(result, activeSlots); 
		return;
	}

	// put these next 2 into the switch and extract methods
	if (result == CreateCustomLink)
	{
		int slotA = activeSlots[0];
		int slotB = activeSlots[1];

		bool isAVca = slotA > PluginConstants::vcaSelectionOffset;
		int trueIdA = isAVca ? slotA - PluginConstants::vcaSelectionOffset : slotA;

		bool isBVca = slotB > PluginConstants::vcaSelectionOffset;
		int trueIdB = isBVca ? slotB - PluginConstants::vcaSelectionOffset : slotB;

		processor.undoManager.beginNewTransaction("Create Custom Link");

		SlotStateHelpers::setCustomLinkedId(processor.apvts.state, slotA, trueIdB, &processor.undoManager);
		SlotStateHelpers::setCustomLinkedIsVca(processor.apvts.state, slotA, isBVca, &processor.undoManager);

		SlotStateHelpers::setCustomLinkedId(processor.apvts.state, slotB, trueIdA, &processor.undoManager);
		SlotStateHelpers::setCustomLinkedIsVca(processor.apvts.state, slotB, isAVca, &processor.undoManager);
		
		juce::Array<int> usedColours;
		auto& state = processor.apvts.state;

		for (int i = 1; i <= PluginConstants::numSlots; ++i) 
		{
			if (SlotStateHelpers::getCustomLinkedId(state, i) != 0)
				usedColours.addIfNotAlreadyThere(SlotStateHelpers::getLinkColourIndex(state, i));
		}
		for (int i = 1; i <= PluginConstants::numVcas; ++i) 
		{
			int vcaTreeId = i + PluginConstants::vcaSelectionOffset;
			if (SlotStateHelpers::getCustomLinkedId(state, vcaTreeId) != 0)
				usedColours.addIfNotAlreadyThere(SlotStateHelpers::getLinkColourIndex(state, vcaTreeId));
		}

		int chosenColour = 0;
		for (int i = 0; i < LinkColours::numColours; ++i) 
		{
			if (!usedColours.contains(i)) 
			{
				chosenColour = i;
				break;
			}
		}

		SlotStateHelpers::setLinkColourIndex(state, slotA, chosenColour, &processor.undoManager);
		SlotStateHelpers::setLinkColourIndex(state, slotB, chosenColour, &processor.undoManager);

		juce::Array<int> slotsToUpdate = { slotA, slotB };
		for (int idx : slotsToUpdate)
		{
			SlotStateHelpers::setLinkPolarityInverse(processor.apvts.state, idx, false, &processor.undoManager);
			SlotStateHelpers::setLinkMaskVolume(processor.apvts.state, idx, true, &processor.undoManager);
			SlotStateHelpers::setLinkMaskMute(processor.apvts.state, idx, true, &processor.undoManager);
			SlotStateHelpers::setLinkMaskSolo(processor.apvts.state, idx, true, &processor.undoManager);
			SlotStateHelpers::setLinkMaskPan(processor.apvts.state, idx, true, &processor.undoManager);
		}
		
		triggerAsyncUpdate();
		return;
	}
	
	if (result == CustomUnlink)
	{
		int slotA = activeSlots[0];
		int targetTrueId = SlotStateHelpers::getCustomLinkedId(processor.apvts.state, slotA);
		bool targetIsVca = SlotStateHelpers::getCustomLinkedIsVca(processor.apvts.state, slotA);
		int slotB = targetIsVca ? targetTrueId + PluginConstants::vcaSelectionOffset : targetTrueId;

		processor.undoManager.beginNewTransaction("Remove Custom Link");

		SlotStateHelpers::setCustomLinkedId(processor.apvts.state, slotA, 0, &processor.undoManager);
		SlotStateHelpers::setCustomLinkedIsVca(processor.apvts.state, slotA, false, &processor.undoManager);
		SlotStateHelpers::removeProp(processor.apvts.state, SlotIDs::linkColourIndex(slotA), &processor.undoManager);

		if (slotB != 0)
		{
			SlotStateHelpers::setCustomLinkedId(processor.apvts.state, slotB, 0, &processor.undoManager);
			SlotStateHelpers::setCustomLinkedIsVca(processor.apvts.state, slotB, false, &processor.undoManager);
			SlotStateHelpers::removeProp(processor.apvts.state, SlotIDs::linkColourIndex(slotB), &processor.undoManager);
		}

		triggerAsyncUpdate();
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
			for (int idx : activeSlots) setSlotStandardGroup(idx, 0, GroupRole::Member);
			break;

		case PromoteLeader:
			if (activeSlots.size() == 1) promoteToGroupLeader(activeSlots[0]);
			break;

		case DemoteMember:
			if (activeSlots.size() == 1) demoteToStandardMember(activeSlots[0]);
			break;

		case ToggleVCA:
			if (activeSlots.size() == 1)
			{
				int id = activeSlots[0];
				int grpId = (id > PluginConstants::vcaSelectionOffset) ?
					(id - PluginConstants::vcaSelectionOffset) :
					SlotStateHelpers::getGroupId(processor.apvts.state, id);

				if (SlotStateHelpers::isValidGroup(grpId))
					toggleVcaMaster(grpId);
			}
			break;

		case ToggleSoloSafe:
			toggleSoloSafe(activeSlots);
			break;

		default:
			break;
	}
}

void PerformanceView::handleClaimSlot(const juce::Array<int>& selectedArr)
{
	processor.undoManager.beginNewTransaction("Claim Slots");

	for (int idx : selectedArr)
	{
		if (!isSlotFullAccess(idx))
		{
			processor.globalSlotRegistry->claimSlot(idx, processor.getInstanceId());

			SlotStateHelpers::setSlotActive(processor.apvts, idx, true);
		}
	}
	selectedItems.deselectAll();
}

void PerformanceView::fillActiveSlots(const juce::Array<int>& selectedArr, juce::Array<int>& activeSlots)
{
	for (int idx : selectedArr)
	{
		if (idx > PluginConstants::vcaSelectionOffset || isSlotFullAccess(idx))
			activeSlots.add(idx);
	}
}

void PerformanceView::handleGroupAssignment(int result, const juce::Array<int>& selectedArr)
{
	int groupId = result - AssignGroupBase;
	for (int idx : selectedArr)
		setSlotStandardGroup(idx, groupId, GroupRole::Member);
}

void PerformanceView::addSoloSafeMenuItem(const juce::Array<int>& activeSlots, juce::PopupMenu& menu) const
{
	if (activeSlots.isEmpty()) return;

	menu.addSeparator();

	if (activeSlots.size() == 1)
	{
		bool isSafe = SlotStateHelpers::isSlotSoloSafe(processor.apvts, activeSlots[0]);
		menu.addItem(ToggleSoloSafe, isSafe ? "Disable Solo Safe" : "Enable Solo Safe");
	}
	else
	{
		menu.addItem(ToggleSoloSafe, "Toggle Solo Safe (Bulk)");
	}
}

void PerformanceView::handleColourAssignment(const juce::Array<int>& selectedArr, int result)
{
	if (selectedArr.size() == 1)
	{
		int colourIdx = result - AssignColourBase;
		int id = selectedArr[0];

		int grpId = (id > PluginConstants::vcaSelectionOffset) ?
			(id - PluginConstants::vcaSelectionOffset) :
			SlotStateHelpers::getGroupId(processor.apvts.state, id);

		if (SlotStateHelpers::isValidGroup(grpId))
		{
			processor.undoManager.beginNewTransaction("Set Group Colour");
			SlotStateHelpers::setGroupColour(processor.apvts.state, grpId, colourIdx, &processor.undoManager);
		}
	}
}

void PerformanceView::doStereoLink(int slotA, int slotB)
{
	if (SlotStateHelpers::isStereoLinked(processor.apvts.state, slotA) ||
		SlotStateHelpers::isStereoLinked(processor.apvts.state, slotB))
	{
		selectedItems.deselectAll();
		return;
	}

	processor.undoManager.beginNewTransaction("Stereo Link");

	auto& state = processor.apvts.state;
	int mainIdx = juce::jmin(slotA, slotB);
	int subIdx = juce::jmax(slotA, slotB);

	setMainSlotProperties(state, mainIdx, subIdx);
	setSubSlotProperties(state, subIdx, mainIdx);

	selectedItems.deselectAll();
}

void PerformanceView::setMainSlotProperties(juce::ValueTree& state, int mainIdx, int subIdx)
{
	SlotStateHelpers::setStereoLinked(state, mainIdx, true, &processor.undoManager);
	SlotStateHelpers::setStereoMain(state, mainIdx, true, &processor.undoManager);
	SlotStateHelpers::setLinkedSlotId(state, mainIdx, subIdx, &processor.undoManager);
}

void PerformanceView::setSubSlotProperties(juce::ValueTree& state, int subIdx, int mainIdx)
{
	SlotStateHelpers::setStereoLinked(state, subIdx, true, &processor.undoManager);
	SlotStateHelpers::setStereoMain(state, subIdx, false, &processor.undoManager);
	SlotStateHelpers::setLinkedSlotId(state, subIdx, mainIdx, &processor.undoManager);

	SlotStateHelpers::setGroupId(state, subIdx, 0, &processor.undoManager);
	SlotStateHelpers::setGroupRole(state, subIdx, GroupRole::Member, &processor.undoManager);
}

void PerformanceView::doStereoUnlink(int slotIdx)
{
	auto& state = processor.apvts.state;
	int linkedIdx = SlotStateHelpers::getLinkedSlotId(state, slotIdx);

	processor.undoManager.beginNewTransaction("Stereo Unlink");

	SlotStateHelpers::unlinkStereoSlot(state, slotIdx, &processor.undoManager);
	if (linkedIdx != -1) 
		SlotStateHelpers::unlinkStereoSlot(state, linkedIdx, &processor.undoManager);

	selectedItems.deselectAll();
}

void PerformanceView::triggerSettling()
{
	isSettling = true;
	startTimer(200);
	triggerAsyncUpdate();
}

void PerformanceView::promoteToGroupLeader(int slotIdx)
{
	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
	if (grpId == 0) return;

	processor.undoManager.beginNewTransaction("Promote Group Leader");

	demoteExistingGroupLeaders(grpId);
	setSlotStandardGroup(slotIdx, grpId, GroupRole::Leader);
}

void PerformanceView::demoteExistingGroupLeaders(int grpId)
{
	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		int otherGrpId = SlotStateHelpers::getGroupId(processor.apvts.state, i);
		GroupRole otherRole = SlotStateHelpers::getGroupRole(processor.apvts.state, i);

		if (otherGrpId == grpId && otherRole == GroupRole::Leader)
		{
			setSlotStandardGroup(i, grpId, GroupRole::Member);
		}
	}
}

void PerformanceView::setSlotStandardGroup(int slotIdx, int groupId, GroupRole role)
{
	auto& state = processor.apvts.state;
	SlotStateHelpers::setGroupId(state, slotIdx, groupId, &processor.undoManager);
	SlotStateHelpers::setGroupRole(state, slotIdx, role, &processor.undoManager);
}

void PerformanceView::demoteToStandardMember(int slotIdx)
{
	processor.undoManager.beginNewTransaction("Demote to Member");
	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotIdx);
	setSlotStandardGroup(slotIdx, grpId, GroupRole::Member);
}

void PerformanceView::toggleVcaMaster(int grpId)
{
	processor.undoManager.beginNewTransaction("Toggle VCA Master");
	bool currentlyEnabled = SlotStateHelpers::isVcaEnabled(processor.apvts, grpId);
	SlotStateHelpers::setParamNormalized(processor.apvts, SlotIDs::vcaEnabled(grpId), currentlyEnabled ? 0.0f : 1.0f);
}

void PerformanceView::reactivateGroupMembers(int grpId)
{
	bool anyChanged = false;

	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		if (SlotStateHelpers::getGroupId(processor.apvts.state, i) == grpId)
		{
			bool isLocallyActive = SlotStateHelpers::isSlotActive(processor.apvts, i);

			if (!isLocallyActive)
			{
				processor.globalSlotRegistry->claimSlot(i, processor.getInstanceId());
				SlotStateHelpers::setSlotActive(processor.apvts, i, true);

				anyChanged = true;
			}
		}
	}

	if (anyChanged)
	{
		triggerAsyncUpdate();
	}
}

void PerformanceView::toggleSoloSafe(const juce::Array<int>& activeSlots)
{
	processor.undoManager.beginNewTransaction("Toggle Solo Safe");

	for (int idx : activeSlots)
	{
		bool isCurrentlySafe = SlotStateHelpers::isSlotSoloSafe(processor.apvts, idx);

		SlotStateHelpers::setSlotSoloSafe(processor.apvts, idx, !isCurrentlySafe);
	}
}

void PerformanceView::paint(juce::Graphics& g)
{
	g.setColour(MyColours::cbBlue);
	g.fillRect(headerArea);
	g.fillRect(footerArea);
}

void PerformanceView::paintOverChildren(juce::Graphics& g)
{
	int activeId = SlotStateHelpers::getActiveStoreId(processor.apvts);

	for (auto* btn : pinnedStoreButtons)
	{
		if (static_cast<int>(btn->getProperties()[PresetTags::StoreIdProp]) == activeId)
		{
			auto b = btn->getBounds();
			g.setColour(hasUnsavedChanges ? juce::Colours::red : juce::Colours::limegreen);
			g.fillEllipse((float)b.getRight() - 8.0f, (float)b.getY() + 2.0f, 6.0f, 6.0f);
		}
	}
}

void PerformanceView::resized()
{
	if (getWidth() <= 0 || getHeight() <= 0)
		return;

	setupAndFillHeader();
	setupAndFillArea();

	juce::String currentSignature = getLayoutSignature();
	bool structureChanged = (currentSignature != lastLayoutSignature);
	bool widthChanged = (getWidth() != lastWindowWidth);

	int expectedPreservedWidth = getCurrentPreservedWidth();

	bool isAutoResize = std::abs(getWidth() - expectedPreservedWidth) <= 2;

	if ((widthChanged && !structureChanged && !isAutoResize) || currentBaselineWidth < 10.0f)
	{
		float baselineWidth = currentBaselineWidth;
		setBaselineWidth(baselineWidth);
	}

	performanceLF.updateGlobalTypography(currentBaselineWidth);

	regularSlotsOnResized(currentBaselineWidth);
	vcaSlotsOnResized(currentBaselineWidth);

	lastWindowWidth = getWidth();
	lastLayoutSignature = currentSignature;
}

void PerformanceView::setBaselineWidth(float& baselineWidth)
{
	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		auto info = getSlotDisplayInfo(i);
		if (info.shouldProcess && info.isVisible)
		{
			baselineWidth = getSlotItem(i)->getWidth() / getSlotWidthMultiplier(info.isStereoMain);
			currentBaselineWidth = baselineWidth;
			return;
		}
	}

	for (int g = 1; g <= PluginConstants::numVcas; ++g)
	{
		if (SlotStateHelpers::isVcaEnabled(processor.apvts, g))
		{
			baselineWidth = getVcaItem(g)->getWidth() / getVcaWidthMultiplier();
			currentBaselineWidth = baselineWidth;
			return;
		}
	}

	baselineWidth = currentBaselineWidth;
}

int PerformanceView::getCurrentPreservedWidth()
{
	float preservedWidth = 0.0f;
	int activeCount = 0;

	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		auto info = getSlotDisplayInfo(i);
		if (info.shouldProcess && info.isVisible)
		{
			preservedWidth += currentBaselineWidth * getSlotWidthMultiplier(info.isStereoMain);
			activeCount++;
		}
	}

	for (int g = 1; g <= PluginConstants::numVcas; ++g)
	{
		if (SlotStateHelpers::isVcaEnabled(processor.apvts, g))
		{
			preservedWidth += currentBaselineWidth * getVcaWidthMultiplier();
			activeCount++;
		}
	}

	if (activeCount == 0)
		return WindowSizeValues::absolutePerfMinWidth;

	return juce::jlimit(getMinWidth(), WindowSizeValues::maxWidth, (int)std::round(preservedWidth));
}

float PerformanceView::getSlotWidthMultiplier(bool isStereoMain) const
{
	return isStereoMain
		? (SlotSizeValues::stereoSlotTargetWidth / SlotSizeValues::monoSlotTargetWidth)
		: 1.0f;
}

float PerformanceView::getVcaWidthMultiplier() const
{
	return SlotSizeValues::vcaSlotTargetWidth / SlotSizeValues::monoSlotTargetWidth;
}

void PerformanceView::regularSlotsOnResized(float baselineWidth)
{
	for (auto* item : slots)
	{
		if (item->isVisible())
		{
			item->setTargetSlotWidth(baselineWidth);
			item->updateTypography();
		}
	}
}

void PerformanceView::vcaSlotsOnResized(float baselineWidth)
{
	for (auto* vcaItem : vcaSlots)
	{
		if (vcaItem->isVisible())
		{
			vcaItem->setTargetSlotWidth(baselineWidth);
			vcaItem->updateTypography();
		}
	}
}

void PerformanceView::setupAndFillHeader()
{
	auto area = getLocalBounds();

	int slotNumberBlueHeight = (int)(area.getHeight() * 0.045f);
	headerArea = area.removeFromTop(topButtonStripHeight + slotNumberBlueHeight);

	auto buttonStrip = headerArea.withHeight(topButtonStripHeight);

	auto presetsArea = buttonStrip.removeFromRight(80).reduced(4, 4);
	presetsButton.setBounds(presetsArea);

	auto leftArea = buttonStrip.removeFromLeft(250);

	auto setupArea = leftArea.removeFromLeft(80).reduced(4, 4);
	setupButton.setBounds(setupArea);

	leftArea.removeFromLeft(20);

	auto labelArea = leftArea.reduced(4, 4);
	activeStoreLabel.setBounds(labelArea);

	xPatchImg.setBounds(headerArea.withHeight(topButtonStripHeight)
		.withSizeKeepingCentre(75, topButtonStripHeight)
		.reduced(4));
}

void PerformanceView::setupAndFillArea()
{
	auto area = getLocalBounds();

	area.removeFromTop(topButtonStripHeight);

	setupAndFillFooter(area);
	juce::FlexBox flexBox = configFlexBox();
	checkAndAddActiveSlots(flexBox);
	flexBox.performLayout(area);
}

void PerformanceView::setupAndFillFooter(juce::Rectangle<int>& area)
{
	footerArea = area.removeFromBottom(40);
	auto areaToUse = footerArea;

	storesButton.setBounds(areaToUse.removeFromLeft(80).reduced(5));

	auto logoArea = areaToUse.removeFromRight(100);
	cbLogo.setBounds(logoArea.withSizeKeepingCentre(50, logoArea.getHeight()).reduced(5));

	updatePinnedButtons();

	setupPinnedStoresFlexBox(areaToUse);
}

void PerformanceView::setupPinnedStoresFlexBox(const juce::Rectangle<int>& areaToUse)
{
	juce::FlexBox storeBox;
	storeBox.flexDirection = juce::FlexBox::Direction::row;
	storeBox.justifyContent = juce::FlexBox::JustifyContent::center;
	storeBox.alignItems = juce::FlexBox::AlignItems::center;

	addPinnedStoreButtons(storeBox);

	storeBox.performLayout(areaToUse);
}

void PerformanceView::addPinnedStoreButtons(juce::FlexBox& storeBox)
{
	for (auto* btn : pinnedStoreButtons)
	{
		storeBox.items.add(juce::FlexItem(*btn)
			.withWidth(50)
			.withHeight(34)
			.withMargin(juce::FlexItem::Margin(0, 3, 0, 3)));
	}
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
	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		auto info = getSlotDisplayInfo(i);
		auto* slot = getSlotItem(i);

		if (info.mode == SlotMode::Disabled || !info.shouldProcess)
		{
			slot->setVisible(false);
			continue;
		}

		slot->setMode(info.mode);
		slot->setVisible(info.isVisible);

		if (info.isVisible)
			addSlotIfActive(true, flexBox, slot, info.isStereoMain);
	}
}

void PerformanceView::hideSlotIfVcaCollapsed(int grpId, bool& shouldShow) const
{
	if (grpId >= 1 && grpId <= PluginConstants::numGroups)
	{
		bool vcaEnabled = SlotStateHelpers::isVcaEnabled(processor.apvts, grpId);
		bool isExpanded = SlotStateHelpers::isVcaExpanded(processor.apvts, grpId);

		if (vcaEnabled && !isExpanded)
		{
			shouldShow = false;
		}
	}
}

void PerformanceView::plotVcaMasters(juce::FlexBox& flexBox)
{
	for (int g = 1; g <= PluginConstants::numVcas; ++g)
	{
		bool vcaEnabled = SlotStateHelpers::isVcaEnabled(processor.apvts, g);
		auto* vca = getVcaItem(g);

		if (vcaEnabled) {
			vca->setVisible(true);
			flexBox.items.add(juce::FlexItem(*vca)
				.withMaxWidth(SlotSizeValues::vcaSlotMaxWidth)
				.withFlex(SlotSizeValues::vcaSlotFlexGrowFactor));
		}
		else {
			vca->setVisible(false);
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

juce::String PerformanceView::getLayoutSignature()
{
	juce::String sig;

	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		auto info = getSlotDisplayInfo(i);
		if (info.shouldProcess && info.isVisible)
		{
			sig += info.isStereoMain ? "S" : "M";
			sig += juce::String(i) + "_";
		}
	}

	for (int g = 1; g <= PluginConstants::numVcas; ++g)
	{
		if (SlotStateHelpers::isVcaEnabled(processor.apvts, g))
		{
			sig += "V" + juce::String(g) + "_";
		}
	}

	return sig;
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
	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		auto info = getSlotDisplayInfo(i);

		if (info.shouldProcess && info.isVisible)
		{
			targetWidth += info.isStereoMain ? SlotSizeValues::stereoSlotTargetWidth : SlotSizeValues::monoSlotTargetWidth;
			activeCount++;
		}
	}
}

void PerformanceView::calculateVcaTargetWidth(int& targetWidth, int& activeCount) const
{
	for (int g = 1; g <= PluginConstants::numVcas; ++g)
	{
		bool vcaEnabled = SlotStateHelpers::isVcaEnabled(processor.apvts, g);

		if (vcaEnabled)
		{
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
	for (int i = 1; i <= PluginConstants::numSlots; ++i) {
		auto info = getSlotDisplayInfo(i);

		if (info.shouldProcess && info.isVisible)
		{
			minWidth += info.isStereoMain ? SlotSizeValues::stereoSlotMinWidth : SlotSizeValues::monoSlotMinWidth;
			activeCount++;
		}
	}
}

void PerformanceView::calcVcaMinWidth(int& minWidth, int& activeCount) const
{
	for (int g = 1; g <= PluginConstants::numVcas; ++g)
	{
		bool vcaEnabled = SlotStateHelpers::isVcaEnabled(processor.apvts, g);
		if (vcaEnabled)
		{
			minWidth += SlotSizeValues::vcaSlotMinWidth;
			activeCount++;
		}
	}
}

PerformanceView::SlotDisplayInfo PerformanceView::getSlotDisplayInfo(int slotId)
{
	SlotDisplayInfo info;

	bool isLocallyActive = SlotStateHelpers::isSlotActive(processor.apvts, slotId);
	info.mode = processor.globalSlotRegistry->getSlotMode(slotId, processor.getInstanceId(), isLocallyActive);

	if (info.mode == SlotMode::Disabled)
		return info;

	bool isLinked = SlotStateHelpers::isStereoLinked(processor.apvts.state, slotId);
	info.isStereoMain = SlotStateHelpers::isStereoMain(processor.apvts.state, slotId);

	if (isLinked && !info.isStereoMain)
	{
		int linkedIdx = SlotStateHelpers::getLinkedSlotId(processor.apvts.state, slotId);
		if (linkedIdx != -1 && !SlotStateHelpers::isSlotActive(processor.apvts, linkedIdx))
		{
			// Orphaned sub-slot, treat it as a standard mono slot
		}
		else
		{
			return info;
		}
	}

	info.shouldProcess = true;

	int grpId = SlotStateHelpers::getGroupId(processor.apvts.state, slotId);
	info.isVisible = true;
	hideSlotIfVcaCollapsed(grpId, info.isVisible);

	return info;
}

bool PerformanceView::isSlotFullAccess(int slotIdx)
{
	bool isLocallyActive = SlotStateHelpers::isSlotActive(processor.apvts, slotIdx);
	return processor.globalSlotRegistry
		->getSlotMode(slotIdx, processor.getInstanceId(), isLocallyActive) == SlotMode::FullAccess;
}

void PerformanceView::updatePinnedButtons()
{
	pinnedStoreButtons.clear();

	auto pinnedIndices = processor.presetManager->getPinnedStores();

	for (int idx : pinnedIndices)
	{
		juce::String storeName = processor.presetManager->getStoreName(idx);
		juce::String buttonText = juce::String(idx) + "\n" + storeName;

		PinnedStoreButton* btn = new PinnedStoreButton(buttonText);
		pinnedStoreButtons.add(btn);

		btn->getProperties().set(PresetTags::StoreIdProp, idx);
		addAndMakeVisible(btn);

		btn->onClick = [this, idx]()
			{
				int currentIndex = SlotStateHelpers::getActiveStoreId(processor.apvts);
				if (currentIndex == idx)
					processor.forceRecallStore(idx);
				else
					SlotStateHelpers::setActiveStoreId(processor.apvts, idx);
			};

		btn->onRightClick = [this, idx](juce::Button* b)
			{
				showPinnedStoreMenu(idx, b);
			};
	}
}

void PerformanceView::handleSelectAll()
{
	selectedItems.deselectAll();

	for (int i = 1; i <= PluginConstants::numSlots; ++i)
	{
		if (isSlotFullAccess(i))
		{
			selectedItems.addToSelection(i);
		}
	}
}

void PerformanceView::handleGroupSelected()
{
	juce::Array<int> activeSlots;
	fillActiveSlots(selectedItems.getItemArray(), activeSlots);

	if (activeSlots.isEmpty()) 
		return;

	int currentGroup = SlotStateHelpers::getGroupId(processor.apvts.state, activeSlots[0]);
	int nextGroup = (currentGroup % PluginConstants::numGroups) + 1;

	processor.undoManager.beginNewTransaction("Group Selected Faders");

	for (int idx : activeSlots)
	{
		setSlotStandardGroup(idx, nextGroup, GroupRole::Member);
	}
}

void PerformanceView::handleRemoveFromGroup()
{
	juce::Array<int> activeSlots;
	fillActiveSlots(selectedItems.getItemArray(), activeSlots);

	if (activeSlots.isEmpty()) return;

	processor.undoManager.beginNewTransaction("Remove Faders from Group");

	for (int idx : activeSlots)
	{
		setSlotStandardGroup(idx, 0, GroupRole::Member);
	}
}

void PerformanceView::handleUndo()
{
	processor.undoManager.undo();
}

void PerformanceView::handleRedo()
{
	processor.undoManager.redo();
}

void PerformanceView::handleSaveActiveStore()
{
	int index = SlotStateHelpers::getActiveStoreId(processor.apvts);
	if (index != PresetConstants::noStore)
	{
		processor.presetManager->saveStore(index, processor.apvts.copyState());
		hasUnsavedChanges = false;
		triggerSettling();
		updateActiveStoreLabel(index);
	}
}