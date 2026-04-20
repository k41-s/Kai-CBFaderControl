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
	configComponents();
	registerIsActiveListener();
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
		addAndMakeVisible(slot);
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

void PerformanceView::configImages()
{
	addAndMakeVisible(logo);
	addAndMakeVisible(xPatchImg);
}

void PerformanceView::registerIsActiveListener()
{
	for (int i = 1; i <= 32; ++i)
	{
		processor.apvts.addParameterListener(SlotIDs::isActive(i), this);
	}
}

PerformanceView::~PerformanceView()
{
	deregisterIsActiveListener();
	setLookAndFeel(nullptr);
}

void PerformanceView::deregisterIsActiveListener()
{
	for (int i = 1; i <= 32; ++i)
	{
		processor.apvts.removeParameterListener(SlotIDs::isActive(i), this);
	}
}

void PerformanceView::parameterChanged(const juce::String& parameterID, float newValue)
{
	triggerAsyncUpdate();
}

void PerformanceView::handleAsyncUpdate()
{
	resized();

	if(onLayoutChangeRequest)
		onLayoutChangeRequest();
}

void PerformanceView::paint(juce::Graphics& g)
{
	g.setColour(MyColours::cbBlue);
	g.fillRect(headerArea);
	g.fillRect(footerArea);
}

void PerformanceView::resized()
{
	setupAndFillArea();
}

void PerformanceView::setHeaderArea()
{
	// This was used for colouring the name and index labels, currently not in use
	auto area = getLocalBounds();
	headerArea = area.removeFromTop(area.getHeight() * 0.075f);
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
	logo.setBounds(logoArea.withSizeKeepingCentre(50, logoArea.getHeight()).reduced(5));
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
		slot->setVisible(isActive);

		addSlotIfActive(isActive, flexBox, slot);
	}
}

void PerformanceView::addSlotIfActive(bool isActive, juce::FlexBox& flexBox, PerformanceSlotItem* slot)
{
	if (isActive)
		flexBox.items.add(juce::FlexItem(*slot)
			.withMaxWidth(120.0f)
			.withFlex(1.0f));
}

int PerformanceView::getIdealWidth()
{
	int activeCount = 0;
	for (int i = 0; i < 32; ++i)
		if (*processor.isActiveParams[i] > 0.5f)
			activeCount++;

	if (activeCount == 0) 
		return WindowSizeValues::minWidth;

	int targetWidth = activeCount * 60;

	return juce::jlimit(
		WindowSizeValues::minWidth,
		WindowSizeValues::maxWidth,
		targetWidth
	);
}