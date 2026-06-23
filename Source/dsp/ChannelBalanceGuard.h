#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

class ChannelBalanceGuard
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        float tc = 0.5f;
        smoothCoeff = std::exp (-(float) maxBlock / ((float) sampleRate * tc));
        reset();
    }

    void reset()
    {
        smoothError = 0.f;
        inputBalDb = 0.f;
    }

    void captureInputBalance (const juce::AudioBuffer<float>& buffer)
    {
        int n = buffer.getNumSamples();
        if (n == 0 || buffer.getNumChannels() < 2) return;

        const float* l = buffer.getReadPointer (0);
        const float* r = buffer.getReadPointer (1);
        float sumLL = 0.f, sumRR = 0.f;
        for (int i = 0; i < n; ++i)
        {
            sumLL += l[i] * l[i];
            sumRR += r[i] * r[i];
        }
        float rL = std::sqrt (sumLL / (float) n);
        float rR = std::sqrt (sumRR / (float) n);

        if (rL > silenceFloor && rR > silenceFloor)
            inputBalDb = 20.f * std::log10 (rL / rR);
        else
            inputBalDb = 0.f;
    }

    void apply (juce::AudioBuffer<float>& buffer)
    {
        int n = buffer.getNumSamples();
        if (n == 0 || buffer.getNumChannels() < 2) return;

        auto* l = buffer.getWritePointer (0);
        auto* r = buffer.getWritePointer (1);

        float sumLL = 0.f, sumRR = 0.f;
        for (int i = 0; i < n; ++i)
        {
            sumLL += l[i] * l[i];
            sumRR += r[i] * r[i];
        }
        float rL = std::sqrt (sumLL / (float) n);
        float rR = std::sqrt (sumRR / (float) n);

        if (rL < silenceFloor && rR < silenceFloor)
        {
            smoothError = smoothCoeff * smoothError;
            return;
        }

        float outBalDb = 0.f;
        if (rL > silenceFloor && rR > silenceFloor)
            outBalDb = 20.f * std::log10 (rL / rR);

        float error = outBalDb - inputBalDb;
        smoothError = smoothCoeff * smoothError + (1.f - smoothCoeff) * error;

        float correctedError = juce::jlimit (-maxCorrectionDb, maxCorrectionDb, smoothError);

        if (std::abs (correctedError) > 0.01f)
        {
            float gL = std::pow (10.f, -correctedError / 40.f);
            float gR = std::pow (10.f,  correctedError / 40.f);
            for (int i = 0; i < n; ++i)
            {
                l[i] *= gL;
                r[i] *= gR;
            }
        }
    }

private:
    float inputBalDb = 0.f;
    float smoothError = 0.f;
    float smoothCoeff = 0.99f;
    static constexpr float maxCorrectionDb = 6.f;
    static constexpr float silenceFloor = 1e-7f;
};
