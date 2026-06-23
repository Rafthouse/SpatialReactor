#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

class NewCenterLock
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        float fc = 120.f;
        float w = 2.f * 3.14159265f * fc / (float) sampleRate;
        lpCoeff = w / (1.f + w);
        scratchBuf.setSize (1, maxBlock, false, false, true);
        reset();
    }

    void reset()
    {
        lpState = 0.f;
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() < 2) return;
        int n = buffer.getNumSamples();

        auto* side = buffer.getWritePointer (1);

        for (int i = 0; i < n; ++i)
        {
            lpState += lpCoeff * (side[i] - lpState);
            side[i] -= lpState;
        }
    }

private:
    float lpCoeff = 0.01f;
    float lpState = 0.f;
    juce::AudioBuffer<float> scratchBuf;
};
