#pragma once
#include <JuceHeader.h>
#include "../../CustomLookAndFeel/PerformanceViewLookFeel/PerformanceViewLookFeel.h"
#include "../../Components/SlotItems/PerformanceSlotItem/PerformanceSlotItem.h"
#include "../../Components/SlotItems/VcaSlotItem/VcaSlotItem.h"
#include "../../../Main/PluginProcessor/PluginProcessor.h"
#include "../../../Utils/BinaryImageComponent/BinaryImageComponent.h"
#include "../../../Utils/Enums/GroupRole.h"
#include "../../../Utils/StateUtils/PresetHelpers.h"
#include "../../Components/PresetLoadDialog/PresetLoadDialog.h"

class PerformanceView :
	public juce::Component,
	public juce::AudioProcessorValueTreeState::Listener,
	public juce::ValueTree::Listener,
	public juce::AsyncUpdater,
	public juce::LassoSource<int>,
	public juce::ChangeListener,
	public juce::Timer
{
public:
	PerformanceView(KaiCBFaderControlAudioProcessor& p);
	~PerformanceView();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;
	void valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged) override;
	void handleAsyncUpdate() override;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override;
	void findLassoItemsInArea(juce::Array<int>& itemsFound, const juce::Rectangle<int>& area) override;
	juce::SelectedItemSet<int>& getLassoSelection() override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;

	void timerCallback() override;

	void paint(juce::Graphics& g) override;
	void paintOverChildren(juce::Graphics& g) override;
	void resized() override;

	void setBaselineWidth(float& baselineWidth);

	void regularSlotsOnResized(float baselineWidth);
	void vcaSlotsOnResized(float baselineWidth);

	int getIdealWidth();
	int getMinWidth();
	int getCurrentPreservedWidth();

	float getSlotWidthMultiplier(bool isStereoMain) const;
	float getVcaWidthMultiplier() const;

	bool getHasUnsavedChanges() const { return hasUnsavedChanges; }

	// Key pressed handlers
	void handleSelectAll();
	void handleGroupSelected();
	void handleRemoveFromGroup();
	void handleUndo();
	void handleRedo();
	void handleSaveActiveStore();

	std::function<void()> onLayoutChangeRequest;
	std::function<void()> onNavigateToSetup;
private:

	struct SlotDisplayInfo {
		bool shouldProcess = false;
		bool isVisible = false;
		bool isStereoMain = false;
		SlotMode mode = SlotMode::Disabled;
	};

	void init();

	// Configuration functions
	void configComponents();
	void createVcaFaderSlots();
	void createFaderSlots();
	void configSetupButton();

	// Presets Helpers
	void configPresetsButton();
	void showPresetsMenu();
	void handleLoadPresetRequest();
	void handleSavePresetRequest();
	void launchLoadPresetChooser();
	void showPresetLoadDialog(std::unique_ptr<juce::XmlElement> xmlRoot);

	// Stores UI
	void configStoresButton();
	void configActiveStoreLabel();
	void updateActiveStoreLabel(int index);
	void handleActiveStoreParamChanged();

	// Store menu functions
	void showStoresMenu();
	void addSetsSubMenu(juce::StringArray& setNames, juce::PopupMenu& menu);
	void addStoreSubMenu(int i, juce::PopupMenu& menu);

	void populateStoreMenuOptions(int i, juce::PopupMenu& storeMenu);
	void showPinnedStoreMenu(int i, juce::Button* btn);

	void handleStoresMenuResult(int result);

	void promptForStoreSetName();
	void handleRecallSetMenuResult(int result);
	void wipeCurrentPins();
	void pinStoresFromSet(juce::StringArray& setNames, int index);
	void handleRemoveSetMenuResult(int result);
	void handleHideSetMenuResult(int setIndex);
	void handleRemoveStoresMenuResult();
	void promptForStoreName(int index);
	void handleStoreRename(juce::AlertWindow* alert, int index);
	void promptForAddMoreStores();
	void handleStoreSaveMenuResult(int result);
	void handleStoreRecallMenuResult(int result);

	// Listener management
	void registerListeners();
	void addRegularSlotListeners();
	void addVcaListeners();

	void deregisterListeners();
	void removeRegularSlotListeners();
	void removeVcaListeners();

	void configImages();

	// Mouse event handlers
	void handleSlotMouseDown(const juce::MouseEvent& e, BaseSlotItem* slot);
	void handleSlotMouseDrag(const juce::MouseEvent& e, BaseSlotItem* slot);
	void handleSlotMouseUp(const juce::MouseEvent& e, BaseSlotItem* slot);
	void bindSelectionEvents(BaseSlotItem* slot);
	void handleBulkToggle(bool isMute, bool newState, PerformanceSlotItem* slotClicked);

	// Context menu functions
	bool handleIsPopupMenuEvent(const juce::MouseEvent& e);
	void showContextMenu();
	void addMenuItems(const juce::Array<int>& selectedArr, juce::PopupMenu& menu);
	void sortSelectedSlots(const juce::Array<int>& selectedArr, juce::Array<int>& activeSlots, juce::Array<int>& readOnlySlots);
	bool isSelectionUnifiedGroup(const juce::Array<int>& normalSlots, const juce::Array<int>& vcaSlots, int& commonGrpId) const;

	void addClaimSlotMenuItem(juce::Array<int>& readOnlySlots, juce::PopupMenu& menu);
	void addStandardMenuOptions(juce::Array<int>& readOnlySlots, juce::PopupMenu& menu, juce::Array<int>& activeSlots);
	void parseSelectedArray(juce::Array<int>& activeSlots, juce::Array<int>& vcaSlots, juce::Array<int>& normalSlots);
	void addNormalSlotItems(juce::Array<int>& normalSlots, juce::PopupMenu& menu, bool containsVca);

	void addLinkMasksMenuItems(juce::Array<int>& activeSlots, juce::PopupMenu& menu);
	bool canCreateCustomLink(int slotA, int slotB) const;
	void addExistingLinkMenuOptions(const juce::Array<int>& activeSlots, juce::PopupMenu& menu);

	void addStereoMenuItems(const juce::Array<int>& selectedArr, juce::PopupMenu& menu) const;

	void addLinkMaskMenu(const juce::Array<int>& activeSlots, const juce::Array<int>& normalSlots, const juce::Array<int>& vcaSlots, juce::PopupMenu& menu) const;
	void setupLinkMaskMenu(const juce::Array<int>& activeSlots, juce::PopupMenu& linkMaskMenu) const;
	void handleLinkMaskResult(int result, const juce::Array<int>& activeSlots);

	void addGroupMenu(const juce::Array<int>& selectedArr, juce::PopupMenu& menu);
	void setupGroupMenu(const juce::Array<int>& selectedArr, juce::PopupMenu& groupMenu) const;

	void addSingleSlotGroupOptions(const juce::Array<int>& selectedArr, juce::PopupMenu& menu);

	void addGroupMemberItems(GroupRole role, juce::PopupMenu& menu);
	void addVcaMenuItem(juce::PopupMenu& menu, int grpId) const;

	void setupAndAddGrpColourMenu(juce::PopupMenu& menu, int grpId);
	void setupGrpColourMenu(int grpId, juce::PopupMenu& colourMenu) const;

	void showPopupMenuIfNotEmpty(juce::PopupMenu& menu, const juce::Array<int>& selectedArr);
	void handlePopupMenuResult(int result, const juce::Array<int>& selectedArr);

	void handleCreateCustomLink(juce::Array<int>& activeSlots);
	void handleCustomUnlink(juce::Array<int>& activeSlots);

	void applyDefaultLinkMasks(int slotId);
	void clearLinkState(int slotId);

	juce::Array<int> getUsedLinkColours(int ignoreSlotA = -1, int ignoreSlotB = -1) const;
	int getAvailableLinkColourIndex() const;
	void setupLinkColourMenu(const juce::Array<int>& activeSlots, juce::PopupMenu& colourMenu) const;
	void handleLinkColourAssignment(const juce::Array<int>& activeSlots, int result);

	void handleClaimSlot(const juce::Array<int>& selectedArr);
	void fillActiveSlots(const juce::Array<int>& selectedArr, juce::Array<int>& activeSlots);

	void handleColourAssignment(const juce::Array<int>& selectedArr, int result);
	void handleGroupAssignment(int result, const juce::Array<int>& selectedArr);

	void addSoloSafeMenuItem(const juce::Array<int>& activeSlots, juce::PopupMenu& menu) const;

	// Stereo link functions
	void doStereoLink(int slotA, int slotB);

	void setMainSlotProperties(juce::ValueTree& state, int mainIdx, int subIdx);
	void setSubSlotProperties(juce::ValueTree& state, int subIdx, int mainIdx);

	void doStereoUnlink(int slotIdx);
	void triggerSettling();

	// Grouping functions
	void setSlotStandardGroup(int slotIdx, int groupId, GroupRole role);
	void promoteToGroupLeader(int slotIdx);
	void demoteExistingGroupLeaders(int grpId);
	void demoteToStandardMember(int slotIdx);
	void toggleVcaMaster(int grpId);
	void reactivateGroupMembers(int grpId);

	void toggleSoloSafe(const juce::Array<int>& activeSlots);

	// Layout and drawing functions
	void setupAndFillHeader();
	void setupAndFillArea();
	void setupAndFillFooter(juce::Rectangle<int>& area);
	void setupPinnedStoresFlexBox(const juce::Rectangle<int>& areaToUse);
	void addPinnedStoreButtons(juce::FlexBox& storeBox);
	juce::FlexBox configFlexBox();
	void checkAndAddActiveSlots(juce::FlexBox& flexBox);
	void plotRegularSlots(juce::FlexBox& flexBox);
	void hideSlotIfVcaCollapsed(int grpId, bool& shouldShow) const;
	void plotVcaMasters(juce::FlexBox& flexBox);
	void addSlotIfActive(bool isActive, juce::FlexBox& flexBox, PerformanceSlotItem* slot, bool isMainStereo);

	SlotDisplayInfo getSlotDisplayInfo(int index);

	bool isSlotFullAccess(int slotIdx);

	// Width calculation helpers
	void calculateRegularSlotTargetWidth(int& targetWidth, int& activeCount);
	void calculateVcaTargetWidth(int& targetWidth, int& activeCount) const;

	void calcRegularSlotMinWidth(int& minWidth, int& activeCount);
	void calcVcaMinWidth(int& minWidth, int& activeCount) const;

	PerformanceSlotItem* getSlotItem(int slotId) const { return slots[slotId - 1]; }
	VcaSlotItem* getVcaItem(int vcaId) const { return vcaSlots[vcaId - 1]; }

	// Stores helpers
	void updatePinnedButtons();
	void createAndAddPinnedButton(int storeIdx);

	juce::String getLayoutSignature();

	// ID Translation Helpers
	bool isVcaSelection(int selectionId) const;
	int getTrueId(int selectionId) const;
	int makeSelectionId(int trueId, bool isVca) const;
	int getLinkedSelectionId(int selectionId) const;

	float currentBaselineWidth = SlotSizeValues::monoSlotTargetWidth;

	juce::String lastLayoutSignature;
	int lastWindowWidth = 0;

	juce::TextButton setupButton{ "Setup" };

	KaiCBFaderControlAudioProcessor& processor;
	PerformanceViewLookFeel performanceLF;

	juce::TextButton presetsButton;

	std::unique_ptr<juce::FileChooser> fileChooser;
	juce::Component::SafePointer<juce::DialogWindow> presetDialogWindow;
	std::unique_ptr<juce::XmlElement> presetToLoadXml;

	juce::OwnedArray<juce::TextButton> pinnedStoreButtons;
	juce::TextButton storesButton;
	juce::Label activeStoreLabel;

	bool hasUnsavedChanges = false;
	bool isSettling = false;

	juce::OwnedArray<PerformanceSlotItem> slots;
	juce::OwnedArray<VcaSlotItem> vcaSlots;

	juce::LassoComponent<int> lasso;
	juce::SelectedItemSet<int> selectedItems;

	BinaryImageComponent cbLogo{ BinaryData::cblogo_png, BinaryData::cblogo_pngSize };
	BinaryImageComponent xPatchImg{ BinaryData::XPatch_png, BinaryData::XPatch_pngSize };

	static constexpr int topButtonStripHeight = 35;

	juce::Rectangle<int> headerArea;
	juce::Rectangle<int> footerArea;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceView)
};