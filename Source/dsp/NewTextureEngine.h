#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "DCBlocker.h"
#include <cmath>

class NewTextureEngine
{
public:
    enum Mode { Silk = 0, Dust, Rust };

    void setMode (int modeIndex) { mode = static_cast<Mode> (juce::jlimit (0, 2, modeIndex)); }

    void prepare (double sampleRate, int maxBlock)
    {
        mode = Silk;
        drySide.setSize (1, maxBlock, false, false, true);
        dcBlock.prepare (sampleRate);

        oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
            1, 2,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            false, true);
        oversampler->initProcessing ((size_t) maxBlock);
    }

    void reset()
    {
        dcBlock.reset();
        if (oversampler) oversampler->reset();
    }

    void process (juce::AudioBuffer<float>& buffer, float depth)
    {
        if (depth <= 0.f || buffer.getNumChannels() < 2 || oversampler == nullptr)
            return;

        int n = buffer.getNumSamples();
        if (n == 0) return;

        drySide.copyFrom (0, 0, buffer.getReadPointer (1), n);

        auto sideBlock = juce::dsp::AudioBlock<float> (buffer).getSingleChannelBlock (1);
        auto osBlock = oversampler->processSamplesUp (sideBlock);
        int osLen = (int) osBlock.getNumSamples();
        auto* osData = osBlock.getChannelPointer (0);

        float drive = 1.f + depth * 2.f;

        switch (mode)
        {
            case Silk:
                for (int i = 0; i < osLen; ++i)
                    osData[i] = std::tanh (osData[i] * drive);
                break;

            case Dust:
                for (int i = 0; i < osLen; ++i)
                {
                    float x = osData[i] * drive;
                    osData[i] = std::tanh (x + 0.3f * std::sin (x * 10.f));
                }
                break;

            case Rust:
                for (int i = 0; i < osLen; ++i)
                {
                    float x = osData[i] * drive;
                    float y = x + 0.3f * x * x * x;
                    osData[i] = std::tanh (y);
                }
                break;
        }

        oversampler->processSamplesDown (sideBlock);

        auto* side = buffer.getWritePointer (1);
        auto* dry = drySide.getReadPointer (0);

        for (int i = 0; i < n; ++i)
            side[i] = dry[i] + (side[i] - dry[i]) * depth;

        dcBlock.processBlock (side, n);
    }

private:
    Mode mode = Silk;
    juce::AudioBuffer<float> drySide;
    DCBlocker dcBlock;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
};
