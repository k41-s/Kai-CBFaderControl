#pragma once
#include<JuceHeader.h>

struct ScopedAtomicSetter
{
    std::atomic<bool>& flag;
    bool previousValue;

    ScopedAtomicSetter(std::atomic<bool>& f, bool newValue) : flag(f)
    {
        // Safely set the new value and remember the old one without copying the atomic object
        previousValue = flag.exchange(newValue);
    }

    ~ScopedAtomicSetter()
    {
        // Safely restore the original value when this struct goes out of scope
        flag.store(previousValue);
    }
};