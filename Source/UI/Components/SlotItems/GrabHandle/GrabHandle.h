#pragma once
#include <JuceHeader.h>

class GrabHandle : public juce::Component
{
public:
    GrabHandle(int associatedSelectionId);

    void paint(juce::Graphics& g) override;

    void mouseDrag(const juce::MouseEvent& e) override;

private:
    int selectionId;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrabHandle)
};