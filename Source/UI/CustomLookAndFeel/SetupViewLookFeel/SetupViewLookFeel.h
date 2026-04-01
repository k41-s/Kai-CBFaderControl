#pragma once
#include <JuceHeader.h>

class SetupViewLookFeel : public juce::LookAndFeel_V4
{
public:
    SetupViewLookFeel();
    
    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;
};