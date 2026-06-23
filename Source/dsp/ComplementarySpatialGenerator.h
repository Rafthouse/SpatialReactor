#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "DCBlocker.h"
#include <cmath>
#include <array>

class ComplementarySpatialGenerator
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        fs = sampleRate;
        static constexpr float c1s[kMaxBands] = { -0.6f, -1.0f, -1.4f, -0.4f };
        static constexpr float c2s[kMaxBands] = {  0.3f,  0.5f,  0.7f,  0.85f };
        for (int i = 0; i < kMaxBands; ++i)
            allpass[i].prepare (sampleRate, c1s[i], c2s[i]);
        for (auto& dc : dcBlock)  dc.prepare (sampleRate);
        hpCoeff = std::exp (-2.f * 3.14159265f * lowCutHz / (float) fs);
        hpState = 0.f;
        reset();
    }

    void reset()
    {
        for (auto& ap : allpass) ap.reset();
        for (auto& dc : dcBlock) dc.reset();
        hpState = 0.f;
    }

    void process (juce::AudioBuffer<float>& buffer, float width, int activeBands)
    {
        if (width <= 0.f || buffer.getNumChannels() < 2)
            return;

        auto* mid  = buffer.getReadPointer (0);
        auto* side = buffer.getWritePointer (1);
        int n = buffer.getNumSamples();
        int bands = juce::jlimit (1, kMaxBands, activeBands);

        for (int i = 0; i < n; ++i)
        {
            float d = mid[i];

            float hp = d - hpState;
            hpState += (1.f - hpCoeff) * (d - hpState);
            d = hp;

            for (int b = 0; b < bands; ++b)
                d = allpass[b].process (d);

            d = dcBlock[0].process (d);

            side[i] += d * width;
        }
    }

private:
    static constexpr int kMaxBands = 4;
    static constexpr float lowCutHz = 200.f;

    struct StaticAllPass
    {
        void prepare (double sr, float c1, float c2)
        {
            coeff1 = c1;
            coeff2 = c2;
            reset();
        }

        void reset()
        {
            w1 = 0.f;
            w2 = 0.f;
        }

        float process (float x)
        {
            float w = x - coeff1 * w1 - coeff2 * w2;
            float y = coeff2 * w + coeff1 * w1 + w2;
            w2 = w1;
            w1 = w;
            return y;
        }

        float coeff1 = 0.f;
        float coeff2 = 0.f;
        float w1 = 0.f, w2 = 0.f;
    };

    std::array<StaticAllPass, kMaxBands> allpass;
    std::array<DCBlocker, 1> dcBlock;
    float hpCoeff = 0.99f;
    float hpState = 0.f;
    double fs = 48000.0;
};
