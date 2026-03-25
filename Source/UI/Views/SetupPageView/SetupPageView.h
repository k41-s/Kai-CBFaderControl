#pragma once
#include<JuceHeader.h>
#include "../SlotConfigItem/SlotConfigItem.h"

class SetupPageView : public juce::Component
{
public:
	void configLocalIpLabel();
	void configLabelEditorPair(juce::String lblTxt,
		juce::Label& label,
		juce::String editorTxt,
		juce::TextEditor& editor);
	void configGridContainer();
	void configGrid();

	void configComponents();

	SetupPageView();
	~SetupPageView() override;

	void setLabelEditorPairBounds(
		juce::Rectangle<int>& area,
		juce::Label& label,
		juce::TextEditor& editor
	);
	void setupLeftPanel(juce::Rectangle<int>& area);
	void setupSlots(int numColumns, int cellWidth, int cellHeight);
	void setupGrid(juce::Rectangle<int>& area);

	void resized() override;
	void paint(juce::Graphics& g) override;


private:
	
	juce::Label localIpLabel;

	juce::Label targetIpLabel;
	juce::TextEditor targetIpEditor;
	
	juce::Label incomingPortLabel;
	juce::TextEditor incomingPortEditor;
	
	juce::Label outgoingPortLabel;
	juce::TextEditor outgoingPortEditor;

	juce::Label statusLabel;

	juce::Viewport gridViewport;
	juce::Component gridContainer;
	juce::OwnedArray<SlotConfigItem> slotItems;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SetupPageView);
};

