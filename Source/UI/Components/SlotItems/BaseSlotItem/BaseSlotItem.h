#pragma once
#include <JuceHeader.h>
#include "../../../../Main/PluginProcessor/PluginProcessor.h"
#include "../../UIConstants.h"
#include "../../PrecisionSlider/PrecisionSlider.h"

class BaseSlotItem : public juce::Component, public juce::ValueTree::Listener
{
public:
    BaseSlotItem(KaiCBFaderControlAudioProcessor& p, int slotIndex);
    ~BaseSlotItem() override;

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void updateTypography();
    int getIndex() const { return index; }
    void setTargetSlotWidth(int width);

protected:
    virtual void updateNameFromValueTree() = 0;
    virtual void setupSlotBounds() = 0;

    void configBaseVolumeFader();
    void configBaseValueLabel();
    void preSeedSlider(juce::RangedAudioParameter* param);

    void configBaseMuteAttachment(const juce::String& paramID);
    void configBaseVolumeAttachment(const juce::String& paramID);

    void updateValueLabel();
    void setupBottomArea(juce::Rectangle<int>& area, int currentWidth);
    void setupFaderBounds(juce::Rectangle<int>& remainingArea);

    void configBaseGroupLabels(std::function<int()> getGroupIdFunc);
    void updateBaseGroupState(int grpId, bool applyFaderIndicator = false);

    int targetSlotWidth = -1; // -1 means unconstrained by default

    KaiCBFaderControlAudioProcessor& processor;
    int index;

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