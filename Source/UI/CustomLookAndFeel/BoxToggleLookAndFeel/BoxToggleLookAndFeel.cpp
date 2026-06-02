#include "BoxToggleLookAndFeel.h"
#include "../MyColours.h"

void BoxToggleLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    float cornerSize = 5.0f;
    bool isOn = button.getToggleState();

    if (isOn)
    {
        g.setColour(MyColours::cbBlue.withAlpha(0.2f));
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(MyColours::cbBlue);
        g.drawRoundedRectangle(bounds, cornerSize, 2.0f);
    }
    else if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(juce::Colours::grey.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    }
    else
    {
        g.setColour(juce::Colours::grey.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    }

    g.setColour(isOn ? juce::Colours::white : juce::Colours::lightgrey);
    g.setFont(juce::Font(14.0f));
    g.drawText(button.getButtonText(), bounds.reduced(15.0f, 0.0f), juce::Justification::centredLeft, true);
}