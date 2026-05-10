#pragma once
#include <JuceHeader.h>
#include "../UIConstants.h"

class PrecisionSlider : public juce::Slider
{
public:
    PrecisionSlider();

    bool hitTest(int x, int y) override;

    void resized() override;

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    double snapValue(double attemptedValue, DragMode dragMode) override;

    std::function<void()> onResolutionChanged;

private:
    juce::Point<int> dragStartPos;

    bool isDragIntentDetermined = false;
    bool isVerticalDrag = false;
};