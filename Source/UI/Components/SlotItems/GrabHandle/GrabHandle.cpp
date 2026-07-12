#include "GrabHandle.h"

GrabHandle::GrabHandle(int associatedSelectionId) : selectionId(associatedSelectionId)
{
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}

void GrabHandle::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds.reduced(1.0f), 3.0f);

    g.setColour(juce::Colours::white.withAlpha(0.95f));

    float cy = bounds.getCentreY();
    float padX = 4.0f;
    float leftX = padX;
    float rightX = bounds.getWidth() - padX;
    float arrowSize = 3.5f;
    float thickness = 1.5f;

    g.drawLine(leftX, cy, rightX, cy, thickness);

    g.drawLine(leftX, cy, leftX + arrowSize, cy - arrowSize, thickness);
    g.drawLine(leftX, cy, leftX + arrowSize, cy + arrowSize, thickness);

    g.drawLine(rightX, cy, rightX - arrowSize, cy - arrowSize, thickness);
    g.drawLine(rightX, cy, rightX - arrowSize, cy + arrowSize, thickness);
}

void GrabHandle::mouseDrag(const juce::MouseEvent& e)
{
    if (auto* dragContainer = juce::DragAndDropContainer::findParentDragContainerFor(this))
    {
        juce::String dragPayload = "SLOT_DRAG|" + juce::String(selectionId);

        dragContainer->startDragging(dragPayload, this, juce::ScaledImage(), false, nullptr, &e.source);
    }
}