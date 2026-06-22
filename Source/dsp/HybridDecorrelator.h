#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class HybridDecorrelator
{
public:
    void prepare(double sampleRate, int /*samplesPerBlock*/)
    {
        for (auto& ap : allPassFilters)
            ap.prepare();

        for (auto& lfo : lfoPhases)
            lfo = 0.0f;

        fs = sampleRate;
    }

    void process(juce::AudioBuffer<float>& buffer,
                 float width,
                 int   activeBands)
    {
        if (width <= 0.0f)
            return;
        if (buffer.getNumChannels() < 2)
            return;

        auto* mid  = buffer.getReadPointer(0);
        auto* side = buffer.getWritePointer(1);
        auto numSamples = buffer.getNumSamples();

        int bands = juce::jlimit(1, 4, activeBands);

        for (int i = 0; i < numSamples; ++i)
        {
            float decorrelated = mid[i];
            for (int b = 0; b < bands; ++b)
            {
                float lfoRate = 0.08f + b * 0.07f;
                float mod = std::sin(lfoPhases[b]) * 0.35f;
                decorrelated = allPassFilters[b].process(decorrelated, mod);
                lfoPhases[b] += juce::MathConstants<float>::twoPi * lfoRate / (float) fs;
                if (lfoPhases[b] > juce::MathConstants<float>::twoPi)
                    lfoPhases[b] -= juce::MathConstants<float>::twoPi;
            }

            side[i] += decorrelated * width;
        }
    }

private:
    struct ModulatedAllPass
    {
        void prepare() { z = 0.0f; }

        float process(float input, float modulation)
        {
            float g = 0.5f + modulation * 0.4f;
            float y = g * input + z;
            z = input - g * y;
            return y;
        }

    private:
        float z = 0.0f;
    };

    std::array<ModulatedAllPass, 4> allPassFilters;
    std::array<float, 4> lfoPhases{};
    double fs = 44100.0;
};
