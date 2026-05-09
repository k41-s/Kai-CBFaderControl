#pragma once
#include<JuceHeader.h>
#include "../../../Main/PluginProcessor/PluginProcessor.h"
#include "../../Components/SlotConfigItem/SlotConfigItem.h"
#include "../../Components/StatusLED/StatusLED.h"
#include "../../CustomLookAndFeel/SetupViewLookFeel/SetupViewLookFeel.h"
#include "../../../Utils/BinaryImageComponent/BinaryImageComponent.h"

class SetupPageView : 
	public juce::Component,
	public juce::ValueTree::Listener,
	public juce::LassoSource<int>,
	public juce::ChangeListener
{
public:
	SetupPageView(KaiCBFaderControlAudioProcessor& p);
	~SetupPageView() override;

	void resized() override;
	void paint(juce::Graphics& g) override;

	void findLassoItemsInArea(juce::Array<int>& itemsFound, const juce::Rectangle<int>& area) override;
	juce::SelectedItemSet<int>& getLassoSelection() override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;

	void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override;

	std::function<void()> onNavigateToPerformance;
private:
	void init();
	void configLocalIpLabel();
	void configComponents();
	void configLabelEditorPair(juce::String lblTxt,
		juce::Label& label,
		juce::String editorTxt,
		juce::TextEditor& editor);
	void restrictPortEditors();
	void configStatusComponents();

	void configGrid();
	void configGridContainer();
	void setupSlotItem(SlotConfigItem* item, int i);

	void setupSlotMouseEvents(SlotConfigItem* item);
	void processLassoDown(const juce::MouseEvent& e);
	void processLassoDrag(const juce::MouseEvent& e);
	void processLassoUp(const juce::MouseEvent& e, SlotConfigItem* clickedItem = nullptr);

	void configToggleAllBtnText();
	void configNavBtn();
	void configImages();

	void saveNetworkSettings();
	void bindNetworkEditorCallbacks();

	void setLabelEditorPairBounds(
		juce::Rectangle<int>& area,
		juce::Label& label,
		juce::TextEditor& editor
	);
	void setStatusBounds(juce::Rectangle<int>& statusRow);
	void setupLeftPanel(juce::Rectangle<int>& area);
	void placeStatusComponents(juce::Rectangle<int>& area);
	void placeToggleAllBtn(juce::Rectangle<int>& area);
	void placeIpComponents(juce::Rectangle<int>& area);
	void placePortComponents(juce::Rectangle<int>& area);
	void placeNavigateBtn(juce::Rectangle<int>& area);
	void placeImages(juce::Rectangle<int>& area);
	void placeCBLogo(juce::Rectangle<int>& areaToUse);
	void placeXPatchImg(juce::Rectangle<int>& areaToUse);
	void setupSlots(int numColumns, int cellWidth, int cellHeight);
	void setupGrid(juce::Rectangle<int>& area);

	void calculateGridDimensions(int& numColumns, int& numRows) const;

	void refreshControlStates();
	void handleEmptySlots();
	void checkSlotActivationStates(bool& allAreActive, bool& atLeastOneActive);
	void updateToggleAllButton(bool allAreActive);
	void updateNavigateBtn(bool atLeastOneActive);
	void setAllSlotsActive(bool shouldBeActive);

	SlotConfigItem* getSlotItem(int slotId) const { return slotItems[slotId - 1]; }

	juce::TextButton navigateBtn{ "Go to Performance" };

	KaiCBFaderControlAudioProcessor& processor;
	SetupViewLookFeel customLF;
	
	juce::Label localIpLabel;

	juce::Label targetIpLabel;
	juce::TextEditor targetIpEditor;
	
	juce::Label incomingPortLabel;
	juce::TextEditor incomingPortEditor;
	
	juce::Label outgoingPortLabel;
	juce::TextEditor outgoingPortEditor;

	juce::Label statusLabel;
	StatusLED statusLED;

	juce::String SELECT_ALL = "Select All";
	juce::String DESELECT_ALL = "Deselect All";

	juce::ToggleButton toggleAllButton{ SELECT_ALL };

	juce::Viewport gridViewport;
	juce::Component gridContainer;
	juce::OwnedArray<SlotConfigItem> slotItems;

	BinaryImageComponent logo{ BinaryData::cblogo_png, BinaryData::cblogo_pngSize };
	BinaryImageComponent xPatchImg{ BinaryData::XPatch_png, BinaryData::XPatch_pngSize };

	juce::Rectangle<int> footerArea;

	juce::LassoComponent<int> lasso;
	juce::SelectedItemSet<int> selectedItems;
	bool isDeactivatingMode = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SetupPageView);
};

