#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

struct MSMatrix
{
    /** Convert stereo L/R to M/S in-place.
        After this call, channel 0 = Mid, channel 1 = Side.
    */
    void process(juce::AudioBuffer<float>& buffer)
    {
        auto* left  = buffer.getWritePointer(0);
        auto* right = buffer.getWritePointer(1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float mid  = (left[i] + right[i]) * 0.5f;
            float side = (left[i] - right[i]) * 0.5f;
            left[i]  = mid;
            right[i] = side;
        }
    }

    /** Convert M/S back to stereo L/R in-place.
        After this call, channel 0 = Left, channel 1 = Right.
    */
    void inverse(juce::AudioBuffer<float>& buffer)
    {
        auto* mid  = buffer.getWritePointer(0);
        auto* side = buffer.getWritePointer(1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float left  = mid[i] + side[i];
            float right = mid[i] - side[i];
            mid[i]   = left;
            side[i]  = right;
        }
    }
};
