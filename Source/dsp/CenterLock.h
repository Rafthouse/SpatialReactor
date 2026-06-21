#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

/**
    Center Lock — attenuates low-frequency content in the Side channel to keep
    bass and kick anchored in the center.
*/
class CenterLock
{
public:
    void prepare(double sampleRate)
    {
        lowpass.prepare({ sampleRate, (juce::uint32)512, 1 });
        lowpass.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
                sampleRate, 120.0f, 0.707f);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        // FIX: self-defense against mono buffer
        if (buffer.getNumChannels() < 2)
            return;

        auto* side = buffer.getWritePointer(1);
        auto numSamples = buffer.getNumSamples();

        juce::AudioBuffer<float> lf(1, numSamples);
        lf.copyFrom(0, 0, side, numSamples);

        {
            auto lfBlock = juce::dsp::AudioBlock<float>(lf);
            lowpass.process(juce::dsp::ProcessContextReplacing<float>(lfBlock));
        }

        for (int i = 0; i < numSamples; ++i)
            side[i] -= lf.getSample(0, i);
    }

private:
    juce::dsp::IIR::Filter<float> lowpass;
};
