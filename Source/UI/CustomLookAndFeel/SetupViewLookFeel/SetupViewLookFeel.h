#pragma once
#include <JuceHeader.h>

namespace MyColours
{
    const juce::Colour background = juce::Colour::fromRGB(36, 36, 36);
    const juce::Colour txtEditorBackground = juce::Colour::fromRGB(254, 248, 231);
    const juce::Colour black = juce::Colours::black.withAlpha(0.9f);
    const juce::Colour white = juce::Colours::white.withAlpha(0.9f);
}

class SetupViewLookFeel : public juce::LookAndFeel_V4
{
public:
    SetupViewLookFeel();
    
    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;
};