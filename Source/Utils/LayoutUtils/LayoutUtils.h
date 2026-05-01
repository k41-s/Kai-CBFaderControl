#pragma once
#include <JuceHeader.h>

namespace LayoutUtils
{
    /*
        Sets the bounds of a component to fill a target area vertically,
        but restricts its width to a maximum value and centers it horizontally.
    */
    static inline void setCenteredMaxWidthBounds(juce::Component& component, const juce::Rectangle<int>& area, int maxWidth)
    {
        int constrainedWidth = juce::jmin(maxWidth, area.getWidth());
        component.setBounds(area.withSizeKeepingCentre(constrainedWidth, area.getHeight()));
    }
}