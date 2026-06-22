#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class TextureEngine
{
public:
    enum Mode { Silk = 0, Dust, Rust };

    void setMode(int modeIndex)
    {
        mode = static_cast<Mode>(juce::jlimit(0, 2, modeIndex));
    }

    void prepare(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;
        mode = Silk;

        oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
            1, 2,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            false, true);
        oversampler->initProcessing((size_t) samplesPerBlock);
    }

    void process(juce::AudioBuffer<float>& buffer, float depth)
    {
        if (buffer.getNumChannels() < 2)
            return;
        if (oversampler == nullptr)
            return;
        if (depth <= 0.0f)
            return;

        auto numSamples = buffer.getNumSamples();
        if (numSamples == 0) return;

        juce::AudioBuffer<float> drySide(1, numSamples);
        drySide.copyFrom(0, 0, buffer.getReadPointer(1), numSamples);

        auto sideBlock = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(1);
        auto osBlock = oversampler->processSamplesUp(sideBlock);
        int osLen = (int) osBlock.getNumSamples();
        auto* osData = osBlock.getChannelPointer(0);

        float drive = 1.0f + depth * 2.0f;

        switch (mode)
        {
            case Silk:
                for (int i = 0; i < osLen; ++i)
                    osData[i] = std::tanh(osData[i] * drive);
                break;

            case Dust:
                for (int i = 0; i < osLen; ++i)
                {
                    float x = osData[i] * drive;
                    osData[i] = std::tanh(x + 0.3f * std::sin(x * 10.0f));
                }
                break;

            case Rust:
                for (int i = 0; i < osLen; ++i)
                {
                    float x = osData[i] * drive;
                    float y = x + 0.7f * x * x + 0.3f * x * x * x;
                    osData[i] = std::tanh(y);
                }
                break;
        }

        oversampler->processSamplesDown(sideBlock);

        auto* side = buffer.getWritePointer(1);
        auto* dry = drySide.getReadPointer(0);
        for (int i = 0; i < numSamples; ++i)
            side[i] = dry[i] + (side[i] - dry[i]) * depth;
    }

private:
    Mode mode = Silk;
    double currentSampleRate = 44100.0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
};
