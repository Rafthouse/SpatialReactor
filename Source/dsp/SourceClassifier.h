#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

/**
    Heuristic source classifier for M/S-encoded audio.
*/
class SourceClassifier
{
public:
    enum Profile
    {
        Vocal      = 0,
        Instrument = 1,
        Master     = 2,
        Ambient    = 3
    };

    void prepare(double sampleRate, int /*blockSize*/)
    {
        rmsMid.reset(sampleRate, 0.1);
        rmsSide.reset(sampleRate, 0.1);
        corrSmoother.reset(sampleRate, 0.2);

        sideHp.prepare({ sampleRate, (juce::uint32)128, 1 });
        sideHp.coefficients =
            juce::dsp::IIR::Coefficients<float>::makeHighPass(
                sampleRate, 6000.0f, 0.707f);
    }

    int classify(const juce::AudioBuffer<float>& midSideBuffer)
    {
        const auto numSamples = midSideBuffer.getNumSamples();
        if (numSamples == 0) return Master;

        // FIX: self-defense against mono buffer
        if (midSideBuffer.getNumChannels() < 2)
            return Master;

        const auto* m = midSideBuffer.getReadPointer(0);
        const auto* s = midSideBuffer.getReadPointer(1);

        float midRmsLin  = 0.0f;
        float sideRmsLin = 0.0f;
        float corr       = 0.0f;

        for (int i = 0; i < (int)numSamples; ++i)
        {
            midRmsLin  += m[i] * m[i];
            sideRmsLin += s[i] * s[i];
            corr       += m[i] * s[i];
        }

        midRmsLin  = std::sqrt(midRmsLin  / numSamples);
        sideRmsLin = std::sqrt(sideRmsLin / numSamples);
        corr      /= (float)numSamples;

        float midDb  = juce::Decibels::gainToDecibels(midRmsLin);
        float sideDb = juce::Decibels::gainToDecibels(sideRmsLin);

        rmsMid.setTargetValue(midDb);
        float smoothMid  = rmsMid.getNextValue();
        rmsSide.setTargetValue(sideDb);
        float smoothSide = rmsSide.getNextValue();
        corrSmoother.setTargetValue(corr);
        float smoothCorr = corrSmoother.getNextValue();

        // Side high-frequency energy proxy
        juce::AudioBuffer<float> sideCopy(1, (int)numSamples);
        sideCopy.copyFrom(0, 0, s, (int)numSamples);
        {
            auto sideCopyBlock = juce::dsp::AudioBlock<float>(sideCopy);
            sideHp.process(juce::dsp::ProcessContextReplacing<float>(sideCopyBlock));
        }
        float hfEnergy = 0.0f;
        for (int i = 0; i < (int)numSamples; ++i)
            hfEnergy += std::abs(sideCopy.getSample(0, i));
        hfEnergy /= (float)numSamples;
        float hfDb = juce::Decibels::gainToDecibels(hfEnergy);

        // Decision logic
        if (smoothCorr > 0.9f && smoothSide < -20.0f)
            return Vocal;

        if (smoothCorr > 0.7f && smoothSide < -12.0f)
            return Instrument;

        if (smoothCorr < 0.4f && smoothSide > -8.0f)
            return Ambient;

        if (smoothCorr < 0.5f && hfDb > -12.0f)
            return Ambient;

        return Master;
    }

private:
    juce::LinearSmoothedValue<float> rmsMid, rmsSide;
    juce::LinearSmoothedValue<float> corrSmoother;
    juce::dsp::IIR::Filter<float> sideHp;
};
