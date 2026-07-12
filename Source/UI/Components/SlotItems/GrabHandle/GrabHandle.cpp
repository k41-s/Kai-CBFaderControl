#include "GrabHandle.h"

GrabHandle::GrabHandle(int associatedSelectionId) : selectionId(associatedSelectionId)
{
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}

void GrabHandle::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    auto area = getLocalBounds().reduced(4, 2).toFloat();
    float spacing = area.getHeight() / 4.0f;

    for (int i = 1; i <= 3; ++i)
    {
        g.drawHorizontalLine(static_cast<int>(area.getY() + (spacing * i)),
            area.getX(),
            area.getRight());
    }
}

void GrabHandle::mouseDrag(const juce::MouseEvent& e)
{
    if (auto* dragContainer = juce::DragAndDropContainer::findParentDragContainerFor(this))
    {
        juce::String dragPayload = "SLOT_DRAG|" + juce::String(selectionId);

        dragContainer->startDragging(dragPayload, this, juce::ScaledImage(), false, nullptr, &e.source);
    }
}