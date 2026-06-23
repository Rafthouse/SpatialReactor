#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

class LoudnessCompensator
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        float bs = (float) maxBlock;
        float tc = 0.3f;
        smoothCoeff = std::exp (-bs / ((float) sampleRate * tc));

        float gainTc = 0.05f;
        gainSmooth = std::exp (-bs / ((float) sampleRate * gainTc));
        reset();
    }

    void reset()
    {
        smoothInE = 0.f;
        smoothOutE = 0.f;
        currentGain = 1.f;
        currentTrimDb = 0.f;
    }

    void captureInput (const juce::AudioBuffer<float>& buffer)
    {
        inputEnergy = estimateEnergy (buffer);
    }

    void applyCompensation (juce::AudioBuffer<float>& buffer)
    {
        float outE = estimateEnergy (buffer);

        smoothInE  = smoothCoeff * smoothInE  + (1.f - smoothCoeff) * inputEnergy;
        smoothOutE = smoothCoeff * smoothOutE + (1.f - smoothCoeff) * outE;

        float targetGain = 1.f;
        if (smoothInE > silenceFloor && smoothOutE > silenceFloor)
            targetGain = std::sqrt (smoothInE / smoothOutE);

        targetGain = juce::jlimit (minGain, maxGain, targetGain);

        currentGain = gainSmooth * currentGain + (1.f - gainSmooth) * targetGain;
        currentTrimDb = 20.f * std::log10 (std::max (currentGain, 1e-10f));

        if (std::abs (currentGain - 1.f) > 0.001f)
        {
            int n = buffer.getNumSamples();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* d = buffer.getWritePointer (ch);
                for (int i = 0; i < n; ++i)
                    d[i] *= currentGain;
            }
        }
    }

    float getTrimDb() const { return currentTrimDb; }

private:
    float estimateEnergy (const juce::AudioBuffer<float>& buffer)
    {
        int n = buffer.getNumSamples();
        if (n == 0 || buffer.getNumChannels() < 2) return 0.f;

        const float* l = buffer.getReadPointer (0);
        const float* r = buffer.getReadPointer (1);
        float sum = 0.f;
        for (int i = 0; i < n; ++i)
            sum += l[i] * l[i] + r[i] * r[i];

        return sum / (2.f * (float) n);
    }

    float smoothCoeff = 0.99f;
    float gainSmooth = 0.99f;
    float smoothInE = 0.f;
    float smoothOutE = 0.f;
    float inputEnergy = 0.f;
    float currentGain = 1.f;
    float currentTrimDb = 0.f;
    static constexpr float minGain = 0.5f;
    static constexpr float maxGain = 2.f;
    static constexpr float silenceFloor = 1e-10f;
};
