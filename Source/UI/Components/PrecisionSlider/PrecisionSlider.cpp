#include "PrecisionSlider.h"

PrecisionSlider::PrecisionSlider()
{
	setSliderSnapsToMousePosition(false);
}

void PrecisionSlider::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    // Intentionally left blank to override default 4dB JUCE jump
    // The PerformanceSlotItem handles the actual increment logic
}

void PrecisionSlider::mouseDown(const juce::MouseEvent& e)
{
    juce::Slider::mouseDown(e);
    mouseDownPos = e.getPosition();
    mouseDownTime = juce::Time::getMillisecondCounter();
}

void PrecisionSlider::mouseUp(const juce::MouseEvent& e)
{
    juce::Slider::mouseUp(e);

    auto timeElapsed = juce::Time::getMillisecondCounter() - mouseDownTime;
    float distanceThreshold = getHeight() * 0.01f;

    detectStationaryClick(e, distanceThreshold, timeElapsed);
}

void PrecisionSlider::detectStationaryClick(const juce::MouseEvent& e, float distanceThreshold, unsigned int timeElapsed)
{
    if (e.getPosition().getDistanceFrom(mouseDownPos) <= distanceThreshold && timeElapsed <= 300)
    {
        bool isHighRes = getProperties().getWithDefault(UIProperties::isHighRes, UIProperties::defaultHighRes);
        getProperties().set(UIProperties::isHighRes, !isHighRes);

        repaint();

        if (onResolutionChanged != nullptr)
            onResolutionChanged();
    }
}

double PrecisionSlider::snapValue(double attemptedValue, DragMode dragMode)
{
    bool isHighRes = getProperties().getWithDefault(UIProperties::isHighRes, UIProperties::defaultHighRes);
    double interval = isHighRes ? 0.25 : 1.0;

    return juce::roundToInt(attemptedValue / interval) * interval;
}