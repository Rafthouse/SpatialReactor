#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

class NewSourceClassifier
{
public:
    enum Profile { Vocal = 0, Instrument = 1, Master = 2, Ambient = 3 };

    void prepare (double sampleRate, int maxBlock)
    {
        float tc = 0.2f;
        smoothCoeff = std::exp (-1.f / ((float) sampleRate * tc / (float) maxBlock));
        hpBuf.setSize (1, maxBlock, false, false, true);

        float fc = 6000.f;
        float w = 2.f * 3.14159265f * fc / (float) sampleRate;
        hpAlpha = 1.f / (1.f + w);
        reset();
    }

    void reset()
    {
        smoothMidDb = -60.f;
        smoothSideDb = -60.f;
        smoothCorr = 0.5f;
        hpPrev = 0.f;
        hpOut = 0.f;
    }

    int classify (const juce::AudioBuffer<float>& buffer)
    {
        int n = buffer.getNumSamples();
        if (n == 0 || buffer.getNumChannels() < 2) return Master;

        const auto* m = buffer.getReadPointer (0);
        const auto* s = buffer.getReadPointer (1);

        float midE = 0.f, sideE = 0.f, corrSum = 0.f;
        for (int i = 0; i < n; ++i)
        {
            midE  += m[i] * m[i];
            sideE += s[i] * s[i];
            corrSum += m[i] * s[i];
        }

        float midRms  = std::sqrt (midE / (float) n);
        float sideRms = std::sqrt (sideE / (float) n);
        float midDb  = midRms  > 1e-10f ? 20.f * std::log10 (midRms)  : -200.f;
        float sideDb = sideRms > 1e-10f ? 20.f * std::log10 (sideRms) : -200.f;
        float corr = corrSum / (float) n;

        float hfEnergy = 0.f;
        for (int i = 0; i < n; ++i)
        {
            float x = s[i];
            float y = hpAlpha * (hpOut + x - hpPrev);
            hpPrev = x;
            hpOut = y;
            hfEnergy += std::abs (y);
        }
        hfEnergy /= (float) n;
        float hfDb = hfEnergy > 1e-10f ? 20.f * std::log10 (hfEnergy) : -200.f;

        float a = smoothCoeff;
        smoothMidDb  = a * smoothMidDb  + (1.f - a) * midDb;
        smoothSideDb = a * smoothSideDb + (1.f - a) * sideDb;
        smoothCorr   = a * smoothCorr   + (1.f - a) * corr;

        if (smoothCorr > 0.9f && smoothSideDb < -20.f) return Vocal;
        if (smoothCorr > 0.7f && smoothSideDb < -12.f) return Instrument;
        if (smoothCorr < 0.4f && smoothSideDb > -8.f)  return Ambient;
        if (smoothCorr < 0.5f && hfDb > -12.f)         return Ambient;
        return Master;
    }

private:
    float smoothCoeff = 0.99f;
    float smoothMidDb = -60.f, smoothSideDb = -60.f, smoothCorr = 0.5f;
    float hpAlpha = 0.5f;
    float hpPrev = 0.f, hpOut = 0.f;
    juce::AudioBuffer<float> hpBuf;
};
