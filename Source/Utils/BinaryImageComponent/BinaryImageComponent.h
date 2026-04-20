#pragma once
#include <JuceHeader.h>

class BinaryImageComponent : public juce::ImageComponent
{
public:
	BinaryImageComponent(const char* binaryData, int binaryDataSize);
	~BinaryImageComponent() override = default;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinaryImageComponent)
};