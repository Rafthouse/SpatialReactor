#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

class NewCorrelationGovernor
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        float tc = 0.02f;
        smoothCoeff = std::exp (-(float) maxBlock / ((float) sampleRate * tc));
        reset();
    }

    void reset()
    {
        smoothGain = 1.f;
        smoothCorr = 1.f;
    }

    float getCorrelation() const { return smoothCorr; }

    void process (juce::AudioBuffer<float>& buffer, bool safeMode)
    {
        if (buffer.getNumChannels() < 2) return;
        int n = buffer.getNumSamples();
        if (n == 0) return;

        auto* mid  = buffer.getReadPointer (0);
        auto* side = buffer.getWritePointer (1);

        float sumMM = 0.f, sumSS = 0.f;
        for (int i = 0; i < n; ++i)
        {
            sumMM += mid[i] * mid[i];
            sumSS += side[i] * side[i];
        }

        float midE  = sumMM / (float) n;
        float sideE = sumSS / (float) n;
        float totalE = midE + sideE;

        if (totalE < silenceFloor)
        {
            smoothCorr = 1.f;
            return;
        }

        float sideRatio = sideE / (totalE + 1e-12f);
        float corrEstimate = 1.f - 2.f * sideRatio;
        corrEstimate = juce::jlimit (-1.f, 1.f, corrEstimate);

        float alpha = smoothCoeff;
        smoothCorr = alpha * smoothCorr + (1.f - alpha) * corrEstimate;

        float floor = safeMode ? safeFloor : freeFloor;
        if (smoothCorr < floor)
        {
            float maxSideRatio = (1.f - floor) * 0.5f;
            float currentSideRatio = sideE / (totalE + 1e-12f);
            if (currentSideRatio > maxSideRatio && currentSideRatio > 1e-10f)
            {
                float targetGain = std::sqrt (maxSideRatio / currentSideRatio);
                targetGain = juce::jlimit (0.1f, 1.f, targetGain);
                smoothGain = alpha * smoothGain + (1.f - alpha) * targetGain;
            }
            else
            {
                smoothGain = alpha * smoothGain + (1.f - alpha) * 1.f;
            }
        }
        else
        {
            smoothGain = alpha * smoothGain + (1.f - alpha) * 1.f;
        }

        if (smoothGain < 0.999f)
        {
            for (int i = 0; i < n; ++i)
                side[i] *= smoothGain;
        }
    }

private:
    float smoothGain = 1.f;
    float smoothCorr = 1.f;
    float smoothCoeff = 0.99f;
    static constexpr float safeFloor = 0.10f;
    static constexpr float freeFloor = -0.30f;
    static constexpr float silenceFloor = 1e-10f;
};
