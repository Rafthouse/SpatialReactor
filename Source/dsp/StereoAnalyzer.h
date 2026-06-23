#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include <atomic>

struct MeterSnapshot
{
    float rmsL = 0.f, rmsR = 0.f;
    float peakL = 0.f, peakR = 0.f;
    float correlation = 1.f;
    float midEnergy = 0.f;
    float sideEnergy = 0.f;
    float balanceDb = 0.f;
    float loudnessEstimate = 0.f;
};

class StereoAnalyzer
{
public:
    void prepare (double sampleRate, int /*maxBlock*/)
    {
        smoothCoeff = std::exp (-1.f / (float)(sampleRate * 0.15));
        reset();
    }

    void reset()
    {
        smoothRmsL = smoothRmsR = 0.f;
        smoothCorr = 1.f;
        smoothLoudness = 0.f;
    }

    MeterSnapshot analyze (const juce::AudioBuffer<float>& buffer)
    {
        int n = buffer.getNumSamples();
        if (n == 0 || buffer.getNumChannels() < 2) return {};

        const float* l = buffer.getReadPointer (0);
        const float* r = buffer.getReadPointer (1);

        float sumLL = 0.f, sumRR = 0.f, sumLR = 0.f;
        float pkL = 0.f, pkR = 0.f;
        float sumL = 0.f, sumR = 0.f;

        for (int i = 0; i < n; ++i)
        {
            sumLL += l[i] * l[i];
            sumRR += r[i] * r[i];
            sumLR += l[i] * r[i];
            pkL = std::max (pkL, std::abs (l[i]));
            pkR = std::max (pkR, std::abs (r[i]));
            sumL += l[i];
            sumR += r[i];
        }

        float fn = (float) n;
        float rL = std::sqrt (sumLL / fn);
        float rR = std::sqrt (sumRR / fn);

        float denom = std::sqrt (sumLL * sumRR);
        float corr = denom > 1e-12f ? sumLR / denom : 1.f;

        float midE  = (sumLL + sumRR + 2.f * sumLR) * 0.5f / fn;
        float sideE = (sumLL + sumRR - 2.f * sumLR) * 0.5f / fn;

        float combined = std::sqrt ((sumLL + sumRR) * 0.5f / fn);
        float loudDb = combined > 1e-10f ? 20.f * std::log10 (combined) : -200.f;

        float alpha = smoothCoeff;
        smoothRmsL = alpha * smoothRmsL + (1.f - alpha) * rL;
        smoothRmsR = alpha * smoothRmsR + (1.f - alpha) * rR;
        smoothCorr = alpha * smoothCorr + (1.f - alpha) * corr;
        smoothLoudness = alpha * smoothLoudness + (1.f - alpha) * loudDb;

        float balDb = 0.f;
        if (smoothRmsL > 1e-8f && smoothRmsR > 1e-8f)
            balDb = 20.f * std::log10 (smoothRmsL / smoothRmsR);

        MeterSnapshot snap;
        snap.rmsL = smoothRmsL;
        snap.rmsR = smoothRmsR;
        snap.peakL = pkL;
        snap.peakR = pkR;
        snap.correlation = smoothCorr;
        snap.midEnergy = midE;
        snap.sideEnergy = sideE;
        snap.balanceDb = balDb;
        snap.loudnessEstimate = smoothLoudness;
        return snap;
    }

private:
    float smoothCoeff = 0.99f;
    float smoothRmsL = 0.f, smoothRmsR = 0.f;
    float smoothCorr = 1.f;
    float smoothLoudness = 0.f;
};
