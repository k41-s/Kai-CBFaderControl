#include "SetupViewLookFeel.h"

SetupViewLookFeel::SetupViewLookFeel()
{
    setColour(juce::TextEditor::backgroundColourId, MyColours::txtEditorBackground);
    setColour(juce::TextEditor::textColourId, MyColours::black);
    setColour(juce::TextEditor::outlineColourId, MyColours::black);
    setColour(juce::TextEditor::focusedOutlineColourId, MyColours::white);

    setColour(juce::TextButton::textColourOffId, MyColours::white);
    setColour(juce::ToggleButton::tickColourId, MyColours::white);
}

void SetupViewLookFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
{
    if (textEditor.hasKeyboardFocus(true) && !textEditor.isReadOnly())
    {
        g.setColour(textEditor.findColour(juce::TextEditor::focusedOutlineColourId));
        g.drawRect(0, 0, width, height, 2);
    }
    else
    {
        g.setColour(textEditor.findColour(juce::TextEditor::outlineColourId));
        g.drawRect(0, 0, width, height, 1);
    }
}