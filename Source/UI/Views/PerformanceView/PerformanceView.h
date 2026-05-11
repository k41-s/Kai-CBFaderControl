#pragma once
#include <JuceHeader.h>
#include "../../CustomLookAndFeel/PerformanceViewLookFeel/PerformanceViewLookFeel.h"
#include "../../Components/SlotItems/PerformanceSlotItem/PerformanceSlotItem.h"
#include "../../Components/SlotItems/VcaSlotItem/VcaSlotItem.h"
#include "../../../Main/PluginProcessor/PluginProcessor.h"
#include "../../../Utils/BinaryImageComponent/BinaryImageComponent.h"
#include "../../../Utils/Enums/GroupRole.h"

class PerformanceView : 
	public juce::Component,
	public juce::AudioProcessorValueTreeState::Listener,
	public juce::ValueTree::Listener,
	public juce::AsyncUpdater,
	public juce::LassoSource<int>,
	public juce::ChangeListener
{
public:
	PerformanceView(KaiCBFaderControlAudioProcessor& p);
	~PerformanceView();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;
	void handleAsyncUpdate() override;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override;
	void findLassoItemsInArea(juce::Array<int>& itemsFound, const juce::Rectangle<int>& area) override;
	juce::SelectedItemSet<int>& getLassoSelection() override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	void setBaselineWidth(float& baselineWidth);

	void regularSlotsOnResized(float baselineWidth);

	void vcaSlotsOnResized(float baselineWidth);

	int getIdealWidth();
	int getMinWidth();

	std::function<void()> onLayoutChangeRequest;
	std::function<void()> onNavigateToSetup;
private:
	void init();

	// Configuration functions
	void configComponents();
	void createVcaFaderSlots();
	void createFaderSlots();
	void setSlotMouseEvents(PerformanceSlotItem* slot);
	void configSetupButton();

	// Listener management
	void registerListeners();
	void addRegularSlotListeners();
	void addVcaListeners();

	void deregisterListeners();
	void removeRegularSlotListeners();
	void removeVcaListeners();

	void configImages();

	// Mouse event handlers
	void handleSlotMouseDown(const juce::MouseEvent& e, PerformanceSlotItem* slot);
	void handleSlotMouseDrag(const juce::MouseEvent& e, PerformanceSlotItem* slot);
	void handleSlotMouseUp(const juce::MouseEvent& e, PerformanceSlotItem* slot);

	// Context menu functions
	bool handleIsPopupMenuEvent(const juce::MouseEvent& e);
	void showContextMenu();
	void addMenuItems(const juce::Array<int>& selectedArr, juce::PopupMenu& menu);
	void sortSelectedSlots(const juce::Array<int>& selectedArr, juce::Array<int>& activeSlots, juce::Array<int>& readOnlySlots);

	void addClaimSlotMenuItem(juce::Array<int>& readOnlySlots, juce::PopupMenu& menu);
	void addStandardMenuOptions(juce::Array<int>& readOnlySlots, juce::PopupMenu& menu, juce::Array<int>& activeSlots);
	void addStereoMenuItems(const juce::Array<int>& selectedArr, juce::PopupMenu& menu) const;

	void addGroupMenu(const juce::Array<int>& selectedArr, juce::PopupMenu& menu);
	void setupGroupMenu(const juce::Array<int>& selectedArr, juce::PopupMenu& groupMenu) const;

	void addSingleSlotGroupOptions(const juce::Array<int>& selectedArr, juce::PopupMenu& menu);

	void addGroupMemberItems(GroupRole role, juce::PopupMenu& menu);
	void addVcaMenuItem(juce::PopupMenu& menu, int grpId) const;

	void setupAndAddColourMenu(juce::PopupMenu& menu, int grpId);
	void setupColourMenu(int grpId, juce::PopupMenu& colourMenu) const;

	void showPopupMenuIfNotEmpty(juce::PopupMenu& menu, const juce::Array<int>& selectedArr);
	void handlePopupMenuResult(int result, const juce::Array<int>& selectedArr);

	void handleClaimSlot(const juce::Array<int>& selectedArr);
	void fillActiveSlots(const juce::Array<int>& selectedArr, juce::Array<int>& activeSlots);

	void handleColourAssignment(const juce::Array<int>& selectedArr, int result);
	void handleGroupAssignment(int result, const juce::Array<int>& selectedArr);

	// Stereo link functions
	void doStereoLink(int slotA, int slotB);

	void setMainSlotProperties(juce::ValueTree& state, int mainIdx, int subIdx);
	void setSubSlotProperties(juce::ValueTree& state, int subIdx, int mainIdx);

	void doStereoUnlink(int slotIdx);
	void unlinkSlot(juce::ValueTree& state, int idx);

	// Grouping functions
	void setSlotStandardGroup(int slotIdx, int groupId, GroupRole role);
	void promoteToGroupLeader(int slotIdx);
	void demoteExistingGroupLeaders(int grpId);
	void demoteToStandardMember(int slotIdx);
	void toggleVcaMaster(int slotIdx);

	// Layout and drawing functions
	void setHeaderArea();
	void setupAndFillArea();
	void setupAndFillFooter(juce::Rectangle<int>& area);
	juce::FlexBox configFlexBox();
	void checkAndAddActiveSlots(juce::FlexBox& flexBox);
	void plotRegularSlots(juce::FlexBox& flexBox);
	void hideSlotIfVcaCollapsed(int grpId, bool& shouldShow);
	void plotVcaMasters(juce::FlexBox& flexBox);
	void addSlotIfActive(bool isActive, juce::FlexBox& flexBox, PerformanceSlotItem* slot, bool isMainStereo);

	struct SlotDisplayInfo {
		bool shouldProcess = false;
		bool isVisible = false;
		bool isStereoMain = false;
		SlotMode mode = SlotMode::Disabled;
	};
	SlotDisplayInfo getSlotDisplayInfo(int index);

	bool isSlotFullAccess(int slotIdx);

	// Width calculation helpers
	void calculateRegularSlotTargetWidth(int& targetWidth, int& activeCount);
	void calculateVcaTargetWidth(int& targetWidth, int& activeCount) const;

	void calcRegularSlotMinWidth(int& minWidth, int& activeCount);
	void calcVcaMinWidth(int& minWidth, int& activeCount) const;

	PerformanceSlotItem* getSlotItem(int slotId) const { return slots[slotId - 1]; }
	VcaSlotItem* getVcaItem(int vcaId) const { return vcaSlots[vcaId - 1]; }

	juce::TextButton setupButton{ "Setup" };

	KaiCBFaderControlAudioProcessor& processor;
	PerformanceViewLookFeel performanceLF;

	juce::OwnedArray<PerformanceSlotItem> slots;
	juce::OwnedArray<VcaSlotItem> vcaSlots;

	juce::LassoComponent<int> lasso;
	juce::SelectedItemSet<int> selectedItems;

	BinaryImageComponent cbLogo{ BinaryData::cblogo_png, BinaryData::cblogo_pngSize };
	BinaryImageComponent xPatchImg{ BinaryData::XPatch_png, BinaryData::XPatch_pngSize };

	juce::Rectangle<int> headerArea;
	juce::Rectangle<int> footerArea;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformanceView)
};