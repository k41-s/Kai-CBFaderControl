#include "SetupViewLookFeel.h"
#include "../MyColours.h"

SetupViewLookFeel::SetupViewLookFeel()
{
    setColour(juce::TextEditor::backgroundColourId, MyColours::txtEditorBackground);
    setColour(juce::TextEditor::textColourId, MyColours::black);
    setColour(juce::TextEditor::outlineColourId, MyColours::black);
    setColour(juce::TextEditor::focusedOutlineColourId, MyColours::white);
    setColour(juce::TextEditor::highlightedTextColourId, juce::Colours::white);

    setColour(juce::TextButton::textColourOffId, MyColours::white);
    setColour(juce::ToggleButton::tickColourId, MyColours::white);

    setColour(juce::LassoComponent<int>::lassoFillColourId, MyColours::setupViewLassoBackground);
    setColour(juce::LassoComponent<int>::lassoOutlineColourId, MyColours::setupViewLassoOutline);
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

void SetupViewLookFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, 
    const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown
) {
    auto area = button.getLocalBounds().toFloat();
    auto cornerSize = 4.0f;
	juce::Colour bgColour = MyColours::unpressedBtn;

    if (!button.isEnabled())
    {
        drawDisabledBtn(g, bgColour, area, cornerSize);
        return;
    }

    auto fillColour = bgColour;
    setRelevantFillColour(fillColour, isButtonDown, isMouseOverButton);
    drawBtn(g, fillColour, area, cornerSize);
    handleMouseOverButton(isMouseOverButton, isButtonDown, g, area, cornerSize);
}


void SetupViewLookFeel::drawDisabledBtn(juce::Graphics& g, const juce::Colour& backgroundColour, juce::Rectangle<float>& area, float cornerSize)
{
    drawDisabledBtnBackground(g, backgroundColour, area, cornerSize);
    drawDisabledBtnOutline(g, area, cornerSize);
}

void SetupViewLookFeel::drawDisabledBtnBackground(juce::Graphics& g, const juce::Colour& backgroundColour, juce::Rectangle<float>& area, float cornerSize)
{
    g.setColour(backgroundColour.withMultipliedAlpha(0.2f));
    g.fillRoundedRectangle(area, cornerSize);
}

void SetupViewLookFeel::drawDisabledBtnOutline(juce::Graphics& g, juce::Rectangle<float>& area, float cornerSize)
{
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRoundedRectangle(area.reduced(0.5f), cornerSize, 1.0f);
}

void SetupViewLookFeel::setRelevantFillColour(juce::Colour& fillColour, bool isButtonDown, bool isMouseOverButton)
{
    if (isButtonDown)
        fillColour = fillColour.darker(0.3f);
    else if (isMouseOverButton)
        fillColour = fillColour.brighter(0.1f);
}

void SetupViewLookFeel::drawBtn(juce::Graphics& g, juce::Colour& fillColour, juce::Rectangle<float>& area, float cornerSize)
{
    drawBtnBody(g, fillColour, area, cornerSize);
    drawBtnOutline(g, area, cornerSize);
}

void SetupViewLookFeel::drawBtnBody(juce::Graphics& g, const juce::Colour& fillColour, const juce::Rectangle<float>& area, float cornerSize)
{
    g.setColour(fillColour);
    g.fillRoundedRectangle(area, cornerSize);
}

void SetupViewLookFeel::drawBtnOutline(juce::Graphics& g, juce::Rectangle<float>& area, float cornerSize)
{
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(area, cornerSize, 1.0f);
}

void SetupViewLookFeel::handleMouseOverButton(bool isMouseOverButton, bool isButtonDown, juce::Graphics& g, juce::Rectangle<float>& area, float cornerSize)
{
    if (isMouseOverButton && !isButtonDown)
    {
        g.setColour(MyColours::mouseOverButton);
        g.fillRoundedRectangle(area.reduced(1.0f), cornerSize);
    }
}
