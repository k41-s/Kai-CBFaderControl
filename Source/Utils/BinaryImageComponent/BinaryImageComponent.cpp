#include "BinaryImageComponent.h"

BinaryImageComponent::BinaryImageComponent(const char* binaryData, int binaryDataSize)
{
	auto image = juce::ImageCache::getFromMemory(binaryData, binaryDataSize);
	setImage(image, juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);
}
