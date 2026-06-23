#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

class EnergyBudgetMixer
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        float tc = 0.01f;
        smoothCoeff = std::exp (-(float) maxBlock / ((float) sampleRate * tc));
        smoothGain = 1.f;
        preEnergy.resize ((size_t) maxBlock);
    }

    void reset()
    {
        smoothGain = 1.f;
    }

    void captureInput (const juce::AudioBuffer<float>& buffer)
    {
        int n = buffer.getNumSamples();
        if (n == 0 || buffer.getNumChannels() < 2) return;

        const float* m = buffer.getReadPointer (0);
        const float* s = buffer.getReadPointer (1);

        inputMidE = 0.f;
        inputSideE = 0.f;
        for (int i = 0; i < n; ++i)
        {
            inputMidE  += m[i] * m[i];
            inputSideE += s[i] * s[i];
        }
        inputMidE  /= (float) n;
        inputSideE /= (float) n;
    }

    void apply (juce::AudioBuffer<float>& buffer)
    {
        int n = buffer.getNumSamples();
        if (n == 0 || buffer.getNumChannels() < 2) return;

        auto* m = buffer.getWritePointer (0);
        auto* s = buffer.getWritePointer (1);

        float outMidE = 0.f, outSideE = 0.f;
        for (int i = 0; i < n; ++i)
        {
            outMidE  += m[i] * m[i];
            outSideE += s[i] * s[i];
        }
        outMidE  /= (float) n;
        outSideE /= (float) n;

        float eIn  = inputMidE + inputSideE + 1e-12f;
        float eOut = outMidE + outSideE + 1e-12f;

        if (eIn < silenceFloor && eOut < silenceFloor)
        {
            smoothGain = 1.f;
            return;
        }

        float ratio = eOut / eIn;

        float targetGain = 1.f;
        if (ratio > maxGrowth)
            targetGain = std::sqrt (eIn * maxGrowth / eOut);

        targetGain = juce::jlimit (0.25f, 4.f, targetGain);

        float alpha = smoothCoeff;
        smoothGain = alpha * smoothGain + (1.f - alpha) * targetGain;

        if (smoothGain < 0.999f)
        {
            for (int i = 0; i < n; ++i)
            {
                m[i] *= smoothGain;
                s[i] *= smoothGain;
            }
        }
    }

private:
    float inputMidE = 0.f, inputSideE = 0.f;
    float smoothGain = 1.f;
    float smoothCoeff = 0.99f;
    static constexpr float silenceFloor = 1e-10f;
    static constexpr float maxGrowth = 2.0f;
    std::vector<float> preEnergy;
};
