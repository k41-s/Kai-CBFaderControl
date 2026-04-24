#pragma once
#include <JuceHeader.h>
#include "../UIConstants.h"

class PrecisionSlider : public juce::Slider
{
public:
    PrecisionSlider();

    bool hitTest(int x, int y) override;

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    double snapValue(double attemptedValue, DragMode dragMode) override;

    std::function<void()> onResolutionChanged;

private:
    void detectStationaryClick(const juce::MouseEvent& e, float distanceThreshold, unsigned int timeElapsed);

    juce::Point<int> dragStartPos;
    juce::uint32 mouseDownTime = 0;

    bool isDragIntentDetermined = false;
    bool isVerticalDrag = false;
};