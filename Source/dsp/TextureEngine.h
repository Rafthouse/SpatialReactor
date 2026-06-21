#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class TextureEngine
{
public:
    enum Mode { Silk = 0, Dust, Rust };

    void setMode(float textureParam)
    {
        Mode newMode;
        if (textureParam < 0.33f)      newMode = Silk;
        else if (textureParam < 0.66f) newMode = Dust;
        else                           newMode = Rust;

        if (newMode != mode)
        {
            mode = newMode;
            // No allocation here. Oversamplers are pre-allocated in prepare().
            activeOversampler = oversamplers[mode].get();
        }
    }

    void prepare(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;
        mode = Silk;

        int factors[] = { 2, 4, 8 };
        for (int i = 0; i < 3; ++i)
        {
            oversamplers[i] = std::make_unique<juce::dsp::Oversampling<float>>(
                1, factors[i],
                juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
                false, true);
            oversamplers[i]->initProcessing ((size_t) samplesPerBlock);
        }
        activeOversampler = oversamplers[0].get();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        // FIX: self-defense against mono buffer
        if (buffer.getNumChannels() < 2)
            return;

        // FIX #1: nullptr guard — processBlock() may be called before prepareToPlay()
        if (activeOversampler == nullptr)
            return;

        auto* side = buffer.getWritePointer(1);
        auto numSamples = buffer.getNumSamples();
        if (numSamples == 0) return;

        auto sideBlock = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(1);
        auto osBlock = activeOversampler->processSamplesUp(sideBlock);
        int osLen = (int)osBlock.getNumSamples();
        auto* osData = osBlock.getChannelPointer(0);

        switch (mode)
        {
            case Silk:
                for (int i = 0; i < osLen; ++i)
                    osData[i] = std::tanh(osData[i] * 1.2f);
                break;

            case Dust:
                for (int i = 0; i < osLen; ++i)
                {
                    float x = osData[i];
                    float y = std::tanh(x * 2.0f + 0.3f * std::sin(x * 10.0f));
                    osData[i] = y * 0.8f + x * 0.2f;
                }
                break;

            case Rust:
                for (int i = 0; i < osLen; ++i)
                {
                    float x = osData[i];
                    float y = x + 0.7f * x * x + 0.3f * x * x * x;
                    y = std::tanh(y * 1.5f);
                    osData[i] = y;
                }
                break;
        }

        activeOversampler->processSamplesDown(sideBlock);
    }

private:
    Mode mode = Silk;
    double currentSampleRate = 44100.0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversamplers[3];
    juce::dsp::Oversampling<float>* activeOversampler = nullptr;
};
