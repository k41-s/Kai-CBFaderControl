#include "GrabHandle.h"

GrabHandle::GrabHandle(int associatedSelectionId) : selectionId(associatedSelectionId)
{
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}

void GrabHandle::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white.withAlpha(0.95f));

    // 1. Calculate the vertical centre of the slot number label
    // The grab handle spans: slotPadding (5px) + labelHeight + indicatorHeight (12px)
    float slotPadding = 5.0f;
    float indicatorHeight = 12.0f;
    float labelHeight = getHeight() - slotPadding - indicatorHeight;
    float cy = slotPadding + (labelHeight * 0.5f) + 1.5f;

    // 2. Bring the arrows in from the edge to frame the number closely
    float padX = 12.0f;
    float leftX = padX;
    float rightX = getWidth() - padX;

    // 3. Make the arrows pointier (taller than they are wide)
    float arrowWidth = 4.5f;
    float arrowHeight = 3.0f;
    float thickness = 1.5f;

    g.drawLine(leftX, cy, leftX + arrowWidth, cy - arrowHeight, thickness);
    g.drawLine(leftX, cy, leftX + arrowWidth, cy + arrowHeight, thickness);

    g.drawLine(rightX, cy, rightX - arrowWidth, cy - arrowHeight, thickness);
    g.drawLine(rightX, cy, rightX - arrowWidth, cy + arrowHeight, thickness);
}

void GrabHandle::mouseDrag(const juce::MouseEvent& e)
{
    if (auto* dragContainer = juce::DragAndDropContainer::findParentDragContainerFor(this))
    {
        juce::String dragPayload = "SLOT_DRAG|" + juce::String(selectionId);

        dragContainer->startDragging(dragPayload, getParentComponent(), juce::ScaledImage(), false, nullptr, &e.source);
    }
}