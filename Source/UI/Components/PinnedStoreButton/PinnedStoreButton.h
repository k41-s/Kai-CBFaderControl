#pragma once
#include <JuceHeader.h>

class PinnedStoreButton : public juce::TextButton
{
public:
	PinnedStoreButton(const juce::String& text) : juce::TextButton(text) {}

	std::function<void(juce::Button*)> onRightClick;

	inline void mouseDown(const juce::MouseEvent& e) override
	{
		if (e.mods.isPopupMenu() && onRightClick)
		{
			onRightClick(this);
		}
		else
		{
			juce::TextButton::mouseDown(e);
		}
	}
};