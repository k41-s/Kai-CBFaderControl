#pragma once
#include <JuceHeader.h>

class SetupViewLookFeel : public juce::LookAndFeel_V4
{
public:
    SetupViewLookFeel();
    
    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, 
        bool isMouseOverButton, bool isButtonDown) override;
private:
	// Helper methods for drawing the button background
    void drawDisabledBtn(juce::Graphics& g, const juce::Colour& backgroundColour, juce::Rectangle<float>& area, float cornerSize);
    void drawDisabledBtnBackground(juce::Graphics& g, const juce::Colour& backgroundColour, 
        juce::Rectangle<float>& area, float cornerSize);
    void drawDisabledBtnOutline(juce::Graphics& g, juce::Rectangle<float>& area, float cornerSize);
    void setRelevantFillColour(juce::Colour& fillColour, bool isButtonDown, bool isMouseOverButton);
    void drawBtn(juce::Graphics& g, juce::Colour& fillColour, juce::Rectangle<float>& area, float cornerSize);
    void drawBtnBody(juce::Graphics& g, const juce::Colour& fillColour, const juce::Rectangle<float>& area, float cornerSize);
    void drawBtnOutline(juce::Graphics& g, juce::Rectangle<float>& area, float cornerSize);
    void handleMouseOverButton(bool isMouseOverButton, bool isButtonDown, 
        juce::Graphics& g, juce::Rectangle<float>& area, float cornerSize);
};