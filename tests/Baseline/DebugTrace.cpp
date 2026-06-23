#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "../../Source/dsp/EnergyPreservingMS.h"
#include "../../Source/dsp/DCBlocker.h"
#include "../../Source/dsp/ComplementarySpatialGenerator.h"
#include "../../Source/dsp/NewTextureEngine.h"
#include "../../Source/dsp/NewAirEngine.h"
#include "../../Source/dsp/EnergyBudgetMixer.h"
#include "../../Source/dsp/NewCenterLock.h"
#include "../../Source/dsp/NewCorrelationGovernor.h"
#include "../../Source/dsp/LoudnessCompensator.h"
#include "../../Source/dsp/ChannelBalanceGuard.h"
#include "../../Source/dsp/TruePeakLimiter.h"
#include "../../Source/dsp/MacroMapper.h"
#include <cmath>
#include <iostream>
#include <iomanip>

static float blockRms (const float* data, int n)
{
    float sum = 0.f;
    for (int i = 0; i < n; ++i) sum += data[i] * data[i];
    return std::sqrt (sum / (float)n);
}

static float toDb (float lin) { return lin > 1e-10f ? 20.f * std::log10 (lin) : -200.f; }

static bool hasNaN (const float* data, int n)
{
    for (int i = 0; i < n; ++i)
        if (std::isnan (data[i]) || std::isinf (data[i])) return true;
    return false;
}

static void printStage (const std::string& name, const juce::AudioBuffer<float>& buf)
{
    int n = buf.getNumSamples();
    float rL = blockRms (buf.getReadPointer (0), n);
    float rR = blockRms (buf.getReadPointer (1), n);
    bool nL = hasNaN (buf.getReadPointer (0), n);
    bool nR = hasNaN (buf.getReadPointer (1), n);
    std::cout << "  " << std::setw(28) << std::left << name
              << " ch0=" << std::setw(9) << std::right << toDb(rL)
              << "  ch1=" << std::setw(9) << toDb(rR)
              << (nL ? " NaN-0!" : "") << (nR ? " NaN-1!" : "") << "\n";
}

int main ()
{

    constexpr double sr = 48000.0;
    constexpr int bs = 512;

    DCBlocker dcL, dcR;
    dcL.prepare (sr);
    dcR.prepare (sr);

    ComplementarySpatialGenerator spatialGen;
    spatialGen.prepare (sr, bs);

    NewTextureEngine textureEngine;
    textureEngine.prepare (sr, bs);

    NewAirEngine airEngine;
    airEngine.prepare (sr, bs);

    EnergyBudgetMixer energyBudget;
    energyBudget.prepare (sr, bs);

    NewCenterLock centerLock;
    centerLock.prepare (sr, bs);

    NewCorrelationGovernor corrGov;
    corrGov.prepare (sr, bs);

    LoudnessCompensator loudComp;
    loudComp.prepare (sr, bs);

    ChannelBalanceGuard balGuard;
    balGuard.prepare (sr, bs);

    TruePeakLimiter peakLim;
    peakLim.prepare (sr, bs);

    MacroMapper macro;

    std::cout << std::fixed << std::setprecision (2);
    std::cout << "=== Full chain trace: mono 1kHz sine, Amount=0.5 ===\n\n";

    for (int block = 0; block < 10; ++block)
    {
        juce::AudioBuffer<float> buf (2, bs);
        for (int i = 0; i < bs; ++i)
        {
            float v = 0.5f * std::sin (2.f * juce::MathConstants<float>::pi * 1000.f
                                        * (float)(block * bs + i) / (float)sr);
            buf.setSample (0, i, v);
            buf.setSample (1, i, v);
        }

        std::cout << "--- Block " << block << " ---\n";
        printStage ("Input", buf);

        dcL.processBlock (buf.getWritePointer(0), bs);
        dcR.processBlock (buf.getWritePointer(1), bs);
        printStage ("DC block", buf);

        loudComp.captureInput (buf);
        balGuard.captureInputBalance (buf);

        EnergyPreservingMS::encode (buf);
        printStage ("M/S encode", buf);

        macro.update (0.5f, 0, true);
        energyBudget.captureInput (buf);

        float widthT = macro.getWidthTarget();
        spatialGen.process (buf, widthT, 3);
        printStage ("Spatial gen", buf);

        float texT = macro.getTextureTarget();
        textureEngine.setMode (0);
        textureEngine.process (buf, texT);
        printStage ("Texture", buf);

        float airT = macro.getAirTarget();
        airEngine.process (buf, airT);
        printStage ("Air", buf);

        energyBudget.apply (buf);
        printStage ("Energy budget", buf);

        centerLock.process (buf);
        printStage ("Center lock", buf);

        corrGov.process (buf, true);
        printStage ("Corr governor", buf);

        EnergyPreservingMS::decode (buf);
        printStage ("M/S decode", buf);

        balGuard.apply (buf);
        printStage ("Balance guard", buf);

        loudComp.applyCompensation (buf);
        printStage ("Loudness comp", buf);

        peakLim.process (buf);
        printStage ("True-peak lim", buf);

        std::cout << "\n";
    }

    return 0;
}
