#pragma once
#include <JuceHeader.h>
#include "../../../../Main/PluginProcessor/PluginProcessor.h"
#include "../../UIConstants.h"
#include "../../PrecisionSlider/PrecisionSlider.h"
#include "../GrabHandle/GrabHandle.h"

class BaseSlotItem : public juce::Component, public juce::ValueTree::Listener
{
public:
    BaseSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex);
    ~BaseSlotItem() override;

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void updateTypography();
    int getIndex() const { return index; }
    void setTargetSlotWidth(int width);

    virtual int getSelectionId() const = 0;
    void setSelected(bool selected);
    bool getSelected() const { return isSelected; }

    virtual void setEditMode(bool isEditModeActive);

    std::function<void(const juce::MouseEvent&, BaseSlotItem*)> onBackgroundMouseDown;
    std::function<void(const juce::MouseEvent&, BaseSlotItem*)> onBackgroundMouseDrag;
    std::function<void(const juce::MouseEvent&, BaseSlotItem*)> onBackgroundMouseUp;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

protected:
    virtual void updateNameFromValueTree() = 0;
    virtual void setupSlotBounds() = 0;

    void valueTreeRedirected(juce::ValueTree& treeWhoseReferenceHasChanged) override;
    virtual void refreshAllVisuals() = 0;

    void configBaseVolumeFader();
    void configBaseValueLabel();
    void preSeedSlider(const juce::String& paramId);

    void configBaseMuteAttachment(const juce::String& paramID);
    void configBaseVolumeAttachment(const juce::String& paramID);

    void updateValueLabel();
    void setupBottomArea(juce::Rectangle<int>& area, int currentWidth);
    void setupFaderBounds(juce::Rectangle<int>& remainingArea);

    void configBaseGroupLabels(std::function<int()> getGroupIdFunc);
    void updateBaseGroupState(int grpId, bool applyFaderIndicator = false);

    void configGrabHandle();
    void injectGrabHandle(juce::Rectangle<int>& area,int slotWidth);

    virtual bool isEventFromButton(juce::Component* comp);

    void paintLinkIndicator(juce::Graphics& g);
    void drawLinkIndicator(juce::Graphics& g, bool isInverse, juce::Colour pairColour);

    bool isSelected = false;

    virtual juce::String getVolumeParamID() const = 0; // Dynamic Id

    int targetSlotWidth = -1; // -1 means unconstrained by default

    KaiCBFaderControlAudioProcessor& processor;
    int index;

    std::unique_ptr<GrabHandle> grabHandle;

    juce::Font sharedFont;

    juce::Label groupLabel;
    juce::Label nameLabel;
    juce::Label indexLabel;
    juce::Label valueLabel;
    juce::Label unitLabel;
    PrecisionSlider volumeFader;
    juce::TextButton muteButton{ UIButtonLabels::mute };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseSlotItem)
};