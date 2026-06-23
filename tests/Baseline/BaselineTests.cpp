#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "../../Source/PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <functional>

static constexpr double kSampleRate = 48000.0;
static constexpr int    kBlockSize  = 512;
static constexpr int    kNumBlocks  = 500;

struct MeasureResult
{
    std::string testName;
    float inputRmsL  = 0.f, inputRmsR  = 0.f;
    float outputRmsL = 0.f, outputRmsR = 0.f;
    float inputPeakL = 0.f, inputPeakR = 0.f;
    float outputPeakL= 0.f, outputPeakR= 0.f;
    float loudnessDeltaDb = 0.f;
    float balanceErrorDb  = 0.f;
    float correlation     = 0.f;
    float monoFoldDiffDb  = 0.f;
    float dcOffsetL = 0.f, dcOffsetR = 0.f;
    float nullDepthDb = 0.f;
    bool  passed = true;
    std::string notes;
};

static float rms (const float* data, int n)
{
    float sum = 0.f;
    for (int i = 0; i < n; ++i) sum += data[i] * data[i];
    return std::sqrt (sum / (float)n);
}

static float peak (const float* data, int n)
{
    float p = 0.f;
    for (int i = 0; i < n; ++i) p = std::max (p, std::abs (data[i]));
    return p;
}

static float dcOffset (const float* data, int n)
{
    float sum = 0.f;
    for (int i = 0; i < n; ++i) sum += data[i];
    return sum / (float)n;
}

static float broadbandCorrelation (const float* l, const float* r, int n)
{
    float sumLR = 0.f, sumLL = 0.f, sumRR = 0.f;
    for (int i = 0; i < n; ++i)
    {
        sumLR += l[i] * r[i];
        sumLL += l[i] * l[i];
        sumRR += r[i] * r[i];
    }
    float denom = std::sqrt (sumLL * sumRR);
    return denom > 1e-12f ? sumLR / denom : 0.f;
}

static float toDb (float linear)
{
    return linear > 1e-10f ? 20.f * std::log10 (linear) : -200.f;
}

using SignalGen = std::function<void(juce::AudioBuffer<float>&, int block)>;

static SignalGen monoSine (float freq)
{
    return [freq](juce::AudioBuffer<float>& buf, int block)
    {
        int offset = block * buf.getNumSamples();
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            float v = 0.5f * std::sin (2.f * juce::MathConstants<float>::pi * freq * (float)(offset + i) / (float)kSampleRate);
            buf.setSample (0, i, v);
            buf.setSample (1, i, v);
        }
    };
}

static SignalGen dualMono ()
{
    return [](juce::AudioBuffer<float>& buf, int block)
    {
        int offset = block * buf.getNumSamples();
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            float v = 0.5f * std::sin (2.f * juce::MathConstants<float>::pi * 1000.f * (float)(offset + i) / (float)kSampleRate);
            buf.setSample (0, i, v);
            buf.setSample (1, i, v);
        }
    };
}

static SignalGen leftOnly ()
{
    return [](juce::AudioBuffer<float>& buf, int block)
    {
        int offset = block * buf.getNumSamples();
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            float v = 0.5f * std::sin (2.f * juce::MathConstants<float>::pi * 1000.f * (float)(offset + i) / (float)kSampleRate);
            buf.setSample (0, i, v);
            buf.setSample (1, i, 0.f);
        }
    };
}

static SignalGen rightOnly ()
{
    return [](juce::AudioBuffer<float>& buf, int block)
    {
        int offset = block * buf.getNumSamples();
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            float v = 0.5f * std::sin (2.f * juce::MathConstants<float>::pi * 1000.f * (float)(offset + i) / (float)kSampleRate);
            buf.setSample (0, i, 0.f);
            buf.setSample (1, i, v);
        }
    };
}

static SignalGen pinkNoise ()
{
    return [](juce::AudioBuffer<float>& buf, int block)
    {
        static juce::Random rng (42);
        static float b0=0, b1=0, b2=0, b3=0, b4=0, b5=0, b6=0;
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            float white = rng.nextFloat() * 2.f - 1.f;
            b0 = 0.99886f * b0 + white * 0.0555179f;
            b1 = 0.99332f * b1 + white * 0.0750759f;
            b2 = 0.96900f * b2 + white * 0.1538520f;
            b3 = 0.86650f * b3 + white * 0.3104856f;
            b4 = 0.55000f * b4 + white * 0.5329522f;
            b5 = -0.7616f * b5 - white * 0.0168980f;
            float pink = (b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f) * 0.11f;
            b6 = white * 0.115926f;
            buf.setSample (0, i, pink * 0.5f);
            buf.setSample (1, i, pink * 0.5f);
        }
    };
}

static SignalGen decorrelatedNoise ()
{
    return [](juce::AudioBuffer<float>& buf, int /*block*/)
    {
        static juce::Random rngL (42), rngR (999);
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            buf.setSample (0, i, (rngL.nextFloat() * 2.f - 1.f) * 0.3f);
            buf.setSample (1, i, (rngR.nextFloat() * 2.f - 1.f) * 0.3f);
        }
    };
}

static MeasureResult runTest (const std::string& name, SignalGen gen,
                               float amount, float density, float texture,
                               int textureMode, float air, bool trust, int profile)
{
    MeasureResult r;
    r.testName = name;

    SpatialReactorAudioProcessor proc;
    proc.prepareToPlay (kSampleRate, kBlockSize);

    proc.apvts.getRawParameterValue ("amount")->store (amount);
    proc.apvts.getRawParameterValue ("density")->store (density);
    proc.apvts.getRawParameterValue ("texture")->store (texture);
    proc.apvts.getRawParameterValue ("textureMode")->store ((float) textureMode);
    proc.apvts.getRawParameterValue ("air")->store (air);
    proc.apvts.getRawParameterValue ("trustMode")->store (trust ? 1.f : 0.f);
    proc.apvts.getRawParameterValue ("profile")->store ((float) profile);

    std::vector<float> allInL, allInR, allOutL, allOutR;
    allInL.reserve (kNumBlocks * kBlockSize);
    allInR.reserve (kNumBlocks * kBlockSize);
    allOutL.reserve (kNumBlocks * kBlockSize);
    allOutR.reserve (kNumBlocks * kBlockSize);

    juce::MidiBuffer midi;

    for (int b = 0; b < kNumBlocks; ++b)
    {
        juce::AudioBuffer<float> buf (2, kBlockSize);
        gen (buf, b);

        for (int i = 0; i < kBlockSize; ++i)
        {
            allInL.push_back (buf.getSample (0, i));
            allInR.push_back (buf.getSample (1, i));
        }

        proc.processBlock (buf, midi);

        for (int i = 0; i < kBlockSize; ++i)
        {
            allOutL.push_back (buf.getSample (0, i));
            allOutR.push_back (buf.getSample (1, i));
        }
    }

    int total = (int) allInL.size();
    int skip  = kBlockSize * 100;
    int measure = total - skip;
    if (measure < kBlockSize) { measure = total; skip = 0; }

    const float* inL  = allInL.data()  + skip;
    const float* inR  = allInR.data()  + skip;
    const float* outL = allOutL.data() + skip;
    const float* outR = allOutR.data() + skip;

    r.inputRmsL  = rms (inL,  measure);
    r.inputRmsR  = rms (inR,  measure);
    r.outputRmsL = rms (outL, measure);
    r.outputRmsR = rms (outR, measure);
    r.inputPeakL = peak (inL,  measure);
    r.inputPeakR = peak (inR,  measure);
    r.outputPeakL= peak (outL, measure);
    r.outputPeakR= peak (outR, measure);

    float inRmsTotal  = std::sqrt ((r.inputRmsL * r.inputRmsL + r.inputRmsR * r.inputRmsR) * 0.5f);
    float outRmsTotal = std::sqrt ((r.outputRmsL * r.outputRmsL + r.outputRmsR * r.outputRmsR) * 0.5f);
    r.loudnessDeltaDb = toDb (outRmsTotal) - toDb (inRmsTotal);

    float inBalDb  = toDb (r.inputRmsL)  - toDb (r.inputRmsR);
    float outBalDb = toDb (r.outputRmsL) - toDb (r.outputRmsR);
    r.balanceErrorDb = (std::isfinite (inBalDb) && std::isfinite (outBalDb))
                       ? outBalDb - inBalDb : 0.f;

    r.correlation = broadbandCorrelation (outL, outR, measure);

    r.dcOffsetL = dcOffset (outL, measure);
    r.dcOffsetR = dcOffset (outR, measure);

    std::vector<float> monoIn (measure), monoOut (measure);
    for (int i = 0; i < measure; ++i)
    {
        monoIn[i]  = (inL[i]  + inR[i])  * 0.5f;
        monoOut[i] = (outL[i] + outR[i]) * 0.5f;
    }
    float monoInRms  = rms (monoIn.data(),  measure);
    float monoOutRms = rms (monoOut.data(), measure);
    r.monoFoldDiffDb = toDb (monoOutRms) - toDb (monoInRms);

    if (amount == 0.f && texture == 0.f && air == 0.f)
    {
        std::vector<float> diffL (measure), diffR (measure);
        for (int i = 0; i < measure; ++i)
        {
            diffL[i] = outL[i] - inL[i];
            diffR[i] = outR[i] - inR[i];
        }
        float diffRmsL = rms (diffL.data(), measure);
        float diffRmsR = rms (diffR.data(), measure);
        float diffTotal = std::sqrt ((diffRmsL * diffRmsL + diffRmsR * diffRmsR) * 0.5f);
        r.nullDepthDb = toDb (diffTotal);
    }

    return r;
}

static void printResult (const MeasureResult& r)
{
    std::cout << std::fixed << std::setprecision (2);
    std::cout << "--- " << r.testName << " ---\n";
    std::cout << "  In  RMS L/R: " << toDb(r.inputRmsL)  << " / " << toDb(r.inputRmsR)  << " dBFS\n";
    std::cout << "  Out RMS L/R: " << toDb(r.outputRmsL) << " / " << toDb(r.outputRmsR) << " dBFS\n";
    std::cout << "  Loudness delta: " << r.loudnessDeltaDb << " dB\n";
    std::cout << "  Balance error:  " << r.balanceErrorDb  << " dB\n";
    std::cout << "  Correlation:    " << r.correlation << "\n";
    std::cout << "  DC offset L/R:  " << std::setprecision(6) << r.dcOffsetL << " / " << r.dcOffsetR << std::setprecision(2) << "\n";
    std::cout << "  Mono fold diff: " << r.monoFoldDiffDb << " dB\n";
    if (r.nullDepthDb > -200.f)
        std::cout << "  Null depth:     " << r.nullDepthDb << " dBFS\n";
    if (!r.notes.empty())
        std::cout << "  NOTES: " << r.notes << "\n";
    std::cout << "\n";
}

int main ()
{
    juce::ScopedJuceInitialiser_GUI init;

    std::cout << "====================================================\n";
    std::cout << "  SPATIAL REACTOR — BASELINE MEASUREMENTS (Phase 1)\n";
    std::cout << "  Sample rate: " << kSampleRate << " Hz, Block: " << kBlockSize << "\n";
    std::cout << "====================================================\n\n";

    std::vector<MeasureResult> results;

    // --- Test 1: Amount=0 Identity ---
    std::cout << "=== TEST GROUP 1: Amount=0 Identity ===\n\n";
    {
        auto r = runTest ("Identity: 1kHz mono sine, Amount=0",
                          monoSine(1000.f), 0.f, 0.5f, 0.f, 0, 0.f, true, 1);
        if (r.nullDepthDb > -40.f) { r.passed = false; r.notes = "FAIL: null depth > -40 dBFS"; }
        printResult (r);
        results.push_back (r);
    }
    {
        auto r = runTest ("Identity: dual-mono, Amount=0",
                          dualMono(), 0.f, 0.5f, 0.f, 0, 0.f, true, 1);
        if (r.nullDepthDb > -40.f) { r.passed = false; r.notes = "FAIL: null depth > -40 dBFS"; }
        printResult (r);
        results.push_back (r);
    }

    // --- Test 2: Loudness increase ---
    std::cout << "=== TEST GROUP 2: Loudness at Amount=0.5 ===\n\n";
    {
        auto r = runTest ("Loudness: 1kHz mono sine, Amount=0.5, SAFE",
                          monoSine(1000.f), 0.5f, 0.5f, 0.f, 0, 0.f, true, 1);
        if (std::abs(r.loudnessDeltaDb) > 0.3f) { r.passed = false; r.notes = "FAIL: loudness delta > 0.3 dB"; }
        printResult (r);
        results.push_back (r);
    }
    {
        auto r = runTest ("Loudness: 1kHz mono sine, Amount=1.0, SAFE",
                          monoSine(1000.f), 1.0f, 0.5f, 0.f, 0, 0.f, true, 1);
        if (std::abs(r.loudnessDeltaDb) > 0.3f) { r.passed = false; r.notes = "FAIL: loudness delta > 0.3 dB"; }
        printResult (r);
        results.push_back (r);
    }
    {
        auto r = runTest ("Loudness: pink noise, Amount=0.7, FREE",
                          pinkNoise(), 0.7f, 0.5f, 0.f, 0, 0.f, false, 3);
        if (std::abs(r.loudnessDeltaDb) > 0.3f) { r.passed = false; r.notes = "FAIL: loudness delta > 0.3 dB"; }
        printResult (r);
        results.push_back (r);
    }

    // --- Test 3: Left-channel drift / Balance ---
    std::cout << "=== TEST GROUP 3: Stereo Balance ===\n\n";
    {
        auto r = runTest ("Balance: pink noise, Amount=0.5, SAFE",
                          pinkNoise(), 0.5f, 0.5f, 0.f, 0, 0.f, true, 1);
        if (std::abs(r.balanceErrorDb) > 0.15f) { r.passed = false; r.notes = "FAIL: balance error > 0.15 dB"; }
        printResult (r);
        results.push_back (r);
    }
    {
        auto r = runTest ("Balance: pink noise, Amount=1.0, Rust",
                          pinkNoise(), 1.0f, 0.5f, 0.5f, 2, 0.f, true, 1);
        if (std::abs(r.balanceErrorDb) > 0.15f) { r.passed = false; r.notes = "FAIL: balance error > 0.15 dB"; }
        printResult (r);
        results.push_back (r);
    }

    // --- Test 4: Channel-swap symmetry ---
    std::cout << "=== TEST GROUP 4: Channel-Swap Symmetry ===\n\n";
    {
        auto r1 = runTest ("Swap-A: left-only 1kHz, Amount=0.5",
                           leftOnly(), 0.5f, 0.5f, 0.f, 0, 0.f, true, 1);
        auto r2 = runTest ("Swap-B: right-only 1kHz, Amount=0.5",
                           rightOnly(), 0.5f, 0.5f, 0.f, 0, 0.f, true, 1);
        float rmsMatchL = std::abs(toDb(r1.outputRmsL) - toDb(r2.outputRmsR));
        float rmsMatchR = std::abs(toDb(r1.outputRmsR) - toDb(r2.outputRmsL));
        r1.notes = "Dominant-ch diff: " + std::to_string(rmsMatchL) + " dB; "
                 + "Quiet-ch diff: " + std::to_string(rmsMatchR) + " dB";
        if (rmsMatchL > 1.0f)
        {
            r1.passed = false;
            r1.notes += " FAIL: dominant channel swap > 1 dB";
        }
        printResult (r1);
        printResult (r2);
        results.push_back (r1);
        results.push_back (r2);
    }

    // --- Test 5: Correlation ---
    std::cout << "=== TEST GROUP 5: Correlation ===\n\n";
    {
        auto r = runTest ("Corr: mono sine, Amount=0.5",
                          monoSine(1000.f), 0.5f, 0.5f, 0.f, 0, 0.f, true, 1);
        if (r.correlation < 0.10f) { r.passed = false; r.notes = "FAIL: corr < 0.10 in SAFE mode"; }
        printResult (r);
        results.push_back (r);
    }
    {
        auto r = runTest ("Corr: mono sine, Amount=1.0, FREE",
                          monoSine(1000.f), 1.0f, 0.5f, 0.5f, 0, 0.5f, false, 3);
        printResult (r);
        results.push_back (r);
    }
    {
        auto r = runTest ("Corr: decorrelated noise, Amount=0.5",
                          decorrelatedNoise(), 0.5f, 0.5f, 0.f, 0, 0.f, true, 1);
        printResult (r);
        results.push_back (r);
    }

    // --- Test 6: Mono fold-down ---
    std::cout << "=== TEST GROUP 6: Mono Fold-Down ===\n\n";
    {
        auto r = runTest ("Mono-fold: 1kHz, Amount=0.5",
                          monoSine(1000.f), 0.5f, 0.5f, 0.f, 0, 0.f, true, 1);
        if (std::abs(r.monoFoldDiffDb) > 0.5f) { r.passed = false; r.notes = "FAIL: mono fold diff > 0.5 dB"; }
        printResult (r);
        results.push_back (r);
    }
    {
        auto r = runTest ("Mono-fold: pink noise, Amount=0.7",
                          pinkNoise(), 0.7f, 0.5f, 0.f, 0, 0.f, true, 3);
        if (std::abs(r.monoFoldDiffDb) > 0.5f) { r.passed = false; r.notes = "FAIL: mono fold diff > 0.5 dB"; }
        printResult (r);
        results.push_back (r);
    }

    // --- Test 7: DC Offset ---
    std::cout << "=== TEST GROUP 7: DC Offset ===\n\n";
    {
        auto r = runTest ("DC: mono sine, Amount=1.0, Rust, texture=0.5",
                          monoSine(1000.f), 1.0f, 0.5f, 0.5f, 2, 0.f, true, 1);
        float dcLdb = toDb(std::abs(r.dcOffsetL));
        float dcRdb = toDb(std::abs(r.dcOffsetR));
        r.notes = "DC L=" + std::to_string(dcLdb) + " R=" + std::to_string(dcRdb) + " dBFS";
        if (dcLdb > -60.f || dcRdb > -60.f) { r.passed = false; r.notes += " FAIL: DC offset > -60 dBFS"; }
        printResult (r);
        results.push_back (r);
    }

    // --- Summary ---
    std::cout << "====================================================\n";
    std::cout << "  SUMMARY\n";
    std::cout << "====================================================\n\n";

    int passed = 0, failed = 0;
    for (auto& r : results)
    {
        if (r.passed) ++passed; else ++failed;
        if (!r.passed)
            std::cout << "  FAIL: " << r.testName << " — " << r.notes << "\n";
    }
    std::cout << "\n  Total: " << results.size() << "  Passed: " << passed << "  Failed: " << failed << "\n\n";

    // --- CSV output ---
    std::ofstream csv ("Tests/Baseline/baseline_measurements.csv");
    if (csv.is_open())
    {
        csv << "Test,InRmsL_dB,InRmsR_dB,OutRmsL_dB,OutRmsR_dB,LoudnessDelta_dB,BalanceError_dB,Correlation,MonoFoldDiff_dB,DC_L,DC_R,NullDepth_dB,Pass\n";
        for (auto& r : results)
        {
            csv << std::fixed << std::setprecision(4)
                << "\"" << r.testName << "\","
                << toDb(r.inputRmsL)  << "," << toDb(r.inputRmsR)  << ","
                << toDb(r.outputRmsL) << "," << toDb(r.outputRmsR) << ","
                << r.loudnessDeltaDb << "," << r.balanceErrorDb << ","
                << r.correlation << "," << r.monoFoldDiffDb << ","
                << r.dcOffsetL << "," << r.dcOffsetR << ","
                << r.nullDepthDb << ","
                << (r.passed ? "PASS" : "FAIL") << "\n";
        }
        csv.close();
        std::cout << "  CSV written to Tests/Baseline/baseline_measurements.csv\n";
    }

    return failed > 0 ? 1 : 0;
}
