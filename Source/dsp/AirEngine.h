#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

/**
    Parallel exciter + Air EQ for the Side channel.
*/
class AirEngine
{
public:
    void prepare(double sampleRate)
    {
        exciterFilter.prepare({ sampleRate, (juce::uint32)512, 1 });
        exciterFilter.coefficients =
            juce::dsp::IIR::Coefficients<float>::makeBandPass(
                sampleRate, 8000.0f, 2.0f);

        airShelf.prepare({ sampleRate, (juce::uint32)512, 1 });
        airShelf.coefficients =
            juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRate, 10000.0f, 0.707f, 1.0f);

        currentSampleRate = sampleRate;
    }

    void process(juce::AudioBuffer<float>& buffer, float airAmount)
    {
        if (airAmount <= 0.0f)
            return;

        // FIX: self-defense against mono buffer
        if (buffer.getNumChannels() < 2)
            return;

        auto* side = buffer.getWritePointer(1);
        auto numSamples = buffer.getNumSamples();

        // 1. Extract 6-20 kHz band from Side
        juce::AudioBuffer<float> band(1, numSamples);
        band.copyFrom(0, 0, side, numSamples);
        {
            auto bandBlock = juce::dsp::AudioBlock<float>(band);
            exciterFilter.process(juce::dsp::ProcessContextReplacing<float>(bandBlock));
        }

        // 2. Generate harmonics via soft clipping
        for (int i = 0; i < numSamples; ++i)
        {
            float x = band.getSample(0, i);
            float harm = std::tanh(x * 3.0f) * 0.5f + x * 0.5f;
            band.setSample(0, i, harm);
        }

        // 3. Parallel mix
        float mix = 0.5f * airAmount;
        for (int i = 0; i < numSamples; ++i)
            side[i] += band.getSample(0, i) * mix;

        // 4. Apply Air EQ shelf on Side only
        {
            auto sideBlock = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(1);
            airShelf.process(juce::dsp::ProcessContextReplacing<float>(sideBlock));
        }
    }

private:
    juce::dsp::IIR::Filter<float> exciterFilter;
    juce::dsp::IIR::Filter<float> airShelf;
    double currentSampleRate = 44100.0;
};
