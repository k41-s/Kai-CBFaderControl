#pragma once
#include<JuceHeader.h>
#include "../../../Main/PluginProcessor/PluginProcessor.h"
#include "../../Components/SlotConfigItem/SlotConfigItem.h"
#include "../../Components/StatusLED/StatusLED.h"
#include "../../CustomLookAndFeel/SetupViewLookFeel/SetupViewLookFeel.h"

class SetupPageView : public juce::Component, public juce::ValueTree::Listener
{
public:
	SetupPageView(KaiCBFaderControlAudioProcessor& p);
	~SetupPageView() override;

	void resized() override;
	void paint(juce::Graphics& g) override;

	void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

private:
	void configLocalIpLabel();
	void configLabelEditorPair(juce::String lblTxt,
		juce::Label& label,
		juce::String editorTxt,
		juce::TextEditor& editor);
	void restrictPortEditors();
	void configGridContainer();
	void configGrid();
	void configStatusComponents();
	void configToggleAllBtnText();
	void configComponents();

	void saveNetworkSettings();
	void bindNetworkEditorCallbacks();

	void setLabelEditorPairBounds(
		juce::Rectangle<int>& area,
		juce::Label& label,
		juce::TextEditor& editor
	);
	void setStatusBounds(juce::Rectangle<int>& statusRow);
	void setupLeftPanel(juce::Rectangle<int>& area);
	void setupSlots(int numColumns, int cellWidth, int cellHeight);
	void setupGrid(juce::Rectangle<int>& area);

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
	void updateToggleAllBtnTxt();
	void setAllSlotsActive(bool shouldBeActive);

	juce::Viewport gridViewport;
	juce::Component gridContainer;
	juce::OwnedArray<SlotConfigItem> slotItems;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SetupPageView);
};

