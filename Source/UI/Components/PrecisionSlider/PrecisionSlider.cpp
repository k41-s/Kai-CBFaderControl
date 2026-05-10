#include "PrecisionSlider.h"

PrecisionSlider::PrecisionSlider()
{
	setSliderSnapsToMousePosition(false);
}

bool PrecisionSlider::hitTest(int x, int y)
{
    float capY = (float)getPositionOfValue(getValue());
    float capX = getWidth() * 0.5f;

    float dx = std::abs(x - capX);
    float dy = std::abs(y - capY);

    float maxDx = getWidth() * 0.5f;
    float maxDy = juce::jmax(25.0f, getHeight() * 0.08f);

    return (dx <= maxDx && dy <= maxDy);
}

void PrecisionSlider::resized()
{
    juce::Slider::resized();

    int trackHeight = juce::jmax(10, getHeight());
    setMouseDragSensitivity(trackHeight);
}

void PrecisionSlider::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    // Intentionally left blank to override default 4dB JUCE jump
    // The PerformanceSlotItem handles the actual increment logic
}

void PrecisionSlider::mouseDown(const juce::MouseEvent& e)
{
    isDragIntentDetermined = false;
    isVerticalDrag = false;
    dragStartPos = e.getPosition();

    juce::Slider::mouseDown(e);
}

void PrecisionSlider::mouseDrag(const juce::MouseEvent& e)
{
    if (!isDragIntentDetermined)
    {
        int dx = std::abs(e.x - dragStartPos.x);
        int dy = std::abs(e.y - dragStartPos.y);

        if (dx > 3 || dy > 3)
        {
            isDragIntentDetermined = true;
            isVerticalDrag = dy > (dx * 1.5f);
        }
    }

    if (isDragIntentDetermined && !isVerticalDrag) return;

    juce::Slider::mouseDrag(e);
}

void PrecisionSlider::mouseUp(const juce::MouseEvent& e)
{
    juce::Slider::mouseUp(e);
}

double PrecisionSlider::snapValue(double attemptedValue, DragMode dragMode)
{
    if (dragMode == juce::Slider::DragMode::notDragging)
        return attemptedValue;

    bool isHighRes = getProperties().getWithDefault(UIProperties::isHighRes, UIProperties::defaultHighRes);
    double interval = isHighRes ? 0.25 : 1.0;

    return juce::roundToInt(attemptedValue / interval) * interval;
}