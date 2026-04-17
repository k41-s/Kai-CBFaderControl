#include <JuceHeader.h>

class PrecisionSlider : public juce::Slider
{
public:
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override {}
};