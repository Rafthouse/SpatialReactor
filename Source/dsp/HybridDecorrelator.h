#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

/**
    Hybrid decorrelator combining modulated all-pass filters and micro-spectral
    modulation across up to 4 frequency bands.

    Operates only on the Side channel to create spatial width without phasiness.
*/
class HybridDecorrelator
{
public:
    void prepare(double sampleRate, int /*samplesPerBlock*/)
    {
        for (auto& ap : allPassFilters)
            ap.prepare(sampleRate);

        for (auto& lfo : lfoPhases)
            lfo = 0.0f;

        lfoRate = 0.1f;
        fs = sampleRate;
    }

    void process(juce::AudioBuffer<float>& buffer,
                 float width,
                 int   /*activeBands*/)
    {
        if (width <= 0.0f)
            return;

        // FIX: self-defense against mono buffer
        if (buffer.getNumChannels() < 2)
            return;

        auto* side = buffer.getWritePointer(1);
        auto numSamples = buffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            float mod = std::sin(lfoPhases[0]) * width * 0.3f;
            side[i] = allPassFilters[0].process(side[i], mod);
            lfoPhases[0] += juce::MathConstants<float>::twoPi * lfoRate / (float)fs;
            if (lfoPhases[0] > juce::MathConstants<float>::twoPi)
                lfoPhases[0] -= juce::MathConstants<float>::twoPi;
        }
    }

private:
    struct ModulatedAllPass
    {
        void prepare(double /*sampleRate*/)
        {
            z = 0.0f;
        }

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
    float lfoRate = 0.1f;
    double fs = 44100.0;
};
