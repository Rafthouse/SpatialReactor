#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "DCBlocker.h"
#include <cmath>

class NewAirEngine
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        fs = sampleRate;
        bandBuf.setSize (1, maxBlock, false, false, true);
        dcBlock.prepare (sampleRate);

        float fc = 8000.f;
        float w0 = 2.f * 3.14159265f * fc / (float) fs;
        float Q = 2.f;
        float alpha = std::sin (w0) / (2.f * Q);
        float a0 = 1.f + alpha;
        bpB0 = (alpha) / a0;
        bpB1 = 0.f;
        bpB2 = (-alpha) / a0;
        bpA1 = (-2.f * std::cos (w0)) / a0;
        bpA2 = (1.f - alpha) / a0;

        reset();
    }

    void reset()
    {
        bpX1 = bpX2 = bpY1 = bpY2 = 0.f;
        dcBlock.reset();
    }

    void process (juce::AudioBuffer<float>& buffer, float airAmount)
    {
        if (airAmount <= 0.f || buffer.getNumChannels() < 2)
            return;

        int n = buffer.getNumSamples();
        if (n == 0) return;

        auto* side = buffer.getWritePointer (1);

        bandBuf.copyFrom (0, 0, side, n);
        auto* band = bandBuf.getWritePointer (0);

        for (int i = 0; i < n; ++i)
        {
            float x = band[i];
            float y = bpB0 * x + bpB1 * bpX1 + bpB2 * bpX2
                    - bpA1 * bpY1 - bpA2 * bpY2;
            bpX2 = bpX1; bpX1 = x;
            bpY2 = bpY1; bpY1 = y;
            band[i] = y;
        }

        for (int i = 0; i < n; ++i)
        {
            float x = band[i];
            band[i] = std::tanh (x * 3.f) * 0.5f + x * 0.5f;
        }

        for (int i = 0; i < n; ++i)
            side[i] += band[i] * airAmount;

        dcBlock.processBlock (side, n);
    }

private:
    double fs = 48000.0;
    juce::AudioBuffer<float> bandBuf;
    DCBlocker dcBlock;
    float bpB0 = 0.f, bpB1 = 0.f, bpB2 = 0.f;
    float bpA1 = 0.f, bpA2 = 0.f;
    float bpX1 = 0.f, bpX2 = 0.f;
    float bpY1 = 0.f, bpY2 = 0.f;
};
