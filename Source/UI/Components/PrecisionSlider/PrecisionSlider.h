#pragma once
#include <JuceHeader.h>
#include "../UIConstants.h"

class PrecisionSlider : public juce::Slider
{
public:
    PrecisionSlider();

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    double snapValue(double attemptedValue, DragMode dragMode) override;

    std::function<void()> onResolutionChanged;

private:
    void detectStationaryClick(const juce::MouseEvent& e, float distanceThreshold, unsigned int timeElapsed);

    juce::Point<int> mouseDownPos;
    juce::uint32 mouseDownTime = 0;
};