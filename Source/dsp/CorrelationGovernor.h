#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

/**
    Correlation governor — gently constrains the Side-to-Mid ratio to prevent
    excessive decorrelation from causing mono-incompatibility.
*/
class CorrelationGovernor
{
public:
    void prepare(double sampleRate, int /*samplesPerBlock*/)
    {
        currentGain = 1.0f;
    }

    void process(juce::AudioBuffer<float>& buffer, bool trustMode)
    {
        // FIX: self-defense against mono buffer
        if (buffer.getNumChannels() < 2)
            return;

        auto* mid  = buffer.getWritePointer(0);
        auto* side = buffer.getWritePointer(1);
        auto numSamples = buffer.getNumSamples();

        float midPeak = 0.0f;
        float sidePeak = 0.0f;
        for (int i = 0; i < numSamples; ++i)
        {
            midPeak  = std::max(midPeak,  std::abs(mid[i]));
            sidePeak = std::max(sidePeak, std::abs(side[i]));
        }

        float maxSideRatio = trustMode ? 0.85f : 1.2f;
        if (midPeak > 1e-6f && sidePeak > midPeak * maxSideRatio)
        {
            float targetGain = (midPeak * maxSideRatio) / sidePeak;
            // Smooth towards target
            currentGain = currentGain + 0.1f * (targetGain - currentGain);
            for (int i = 0; i < numSamples; ++i)
                side[i] *= currentGain;
        }
        else
        {
            // Slowly return to unity
            currentGain = currentGain + 0.01f * (1.0f - currentGain);
        }
    }

private:
    float currentGain = 1.0f;
};
