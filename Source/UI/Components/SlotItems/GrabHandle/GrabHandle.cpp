#include "GrabHandle.h"
#include "../../UIConstants.h"

GrabHandle::GrabHandle(int associatedSelectionId) : selectionId(associatedSelectionId)
{
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}

void GrabHandle::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white.withAlpha(0.95f));

    float slotPadding = 5.0f;
    float indicatorHeight = 12.0f;
    float labelHeight = getHeight() - slotPadding - indicatorHeight;
    float cy = slotPadding + (labelHeight * 0.5f) + 1.5f;

    float padX = 12.0f;
    float leftX = padX;
    float rightX = getWidth() - padX;

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
        juce::String dragPayload = DragAndDropConstants::slotDragPayloadPrefix + juce::String(selectionId);

        dragContainer->startDragging(dragPayload, getParentComponent(), juce::ScaledImage(), false, nullptr, &e.source);
    }
}