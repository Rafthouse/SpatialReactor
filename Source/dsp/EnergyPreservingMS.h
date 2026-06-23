#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

struct EnergyPreservingMS
{
    static constexpr float kInvSqrt2 = 0.7071067811865475f; // 1/sqrt(2)

    static void encode (juce::AudioBuffer<float>& buffer)
    {
        jassert (buffer.getNumChannels() >= 2);
        auto* l = buffer.getWritePointer (0);
        auto* r = buffer.getWritePointer (1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float m = (l[i] + r[i]) * kInvSqrt2;
            float s = (l[i] - r[i]) * kInvSqrt2;
            l[i] = m;
            r[i] = s;
        }
    }

    static void decode (juce::AudioBuffer<float>& buffer)
    {
        jassert (buffer.getNumChannels() >= 2);
        auto* m = buffer.getWritePointer (0);
        auto* s = buffer.getWritePointer (1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float l = (m[i] + s[i]) * kInvSqrt2;
            float r = (m[i] - s[i]) * kInvSqrt2;
            m[i] = l;
            s[i] = r;
        }
    }
};
