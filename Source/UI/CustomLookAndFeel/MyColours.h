#include <JuceHeader.h>

namespace MyColours
{
    const juce::Colour cbBlue = juce::Colour::fromRGB(28, 45, 135);

    const juce::Colour background = juce::Colour(0xFF252422);
    const juce::Colour txtEditorBackground = juce::Colour::fromRGB(254, 248, 231);

    const juce::Colour faderTrack = juce::Colour::fromRGB(20, 20, 20);
    const juce::Colour faderCap = juce::Colours::white.darker(0.1f);

    const juce::Colour black = juce::Colours::black.withAlpha(0.9f);
    const juce::Colour white = juce::Colours::white.withAlpha(0.9f);

    const juce::Colour unpressedBtn = juce::Colour(0xFF2A2A2A);
    const juce::Colour mouseOverButton = juce::Colours::white.withAlpha(0.05f);
	const juce::Colour muteBtnPressed = juce::Colour(0xFFD24242).withAlpha(0.8f);
	const juce::Colour soloBtnPressed = juce::Colour(0xFFF4EB66).withAlpha(0.8f);
}