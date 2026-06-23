#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

class TruePeakLimiter
{
public:
    void prepare (double sampleRate, int /*maxBlock*/)
    {
        float releaseTc = 0.1f;
        releaseCoeff = std::exp (-1.f / ((float) sampleRate * releaseTc));
        reset();
    }

    void reset()
    {
        currentGain = 1.f;
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        int n = buffer.getNumSamples();
        int ch = buffer.getNumChannels();

        for (int i = 0; i < n; ++i)
        {
            float maxSample = 0.f;
            for (int c = 0; c < ch; ++c)
                maxSample = std::max (maxSample, std::abs (buffer.getSample (c, i)));

            float targetGain = 1.f;
            if (maxSample > ceiling)
                targetGain = ceiling / maxSample;

            if (targetGain < currentGain)
                currentGain = targetGain;
            else
                currentGain = releaseCoeff * currentGain + (1.f - releaseCoeff) * targetGain;

            if (currentGain < 0.999f)
            {
                for (int c = 0; c < ch; ++c)
                    buffer.getWritePointer (c)[i] *= currentGain;
            }
        }
    }

private:
    float ceiling = 0.99f;
    float releaseCoeff = 0.999f;
    float currentGain = 1.f;
};
