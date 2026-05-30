#pragma once
#include <JuceHeader.h>

namespace NetworkUtils
{
    static inline juce::String getLocalIpAddress()
    {
        auto addresses = juce::IPAddress::getAllAddresses();

        for (const auto& addr : addresses)
        {
            auto ipString = addr.toString();
            if (ipString.contains(".") && ipString != NetworkConstants::defaultTargetIp)
                return ipString;
        }
        return NetworkConstants::defaultTargetIp;
    }
}