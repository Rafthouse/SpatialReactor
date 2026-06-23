#include <cstdio>
#include <cmath>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Source/PluginProcessor.h"

int main()
{
    juce::ScopedJuceInitialiser_GUI init;

    printf("=== SPATIAL REACTOR AUDIO CHECK ===\n\n");

    auto* proc = new SpatialReactorAudioProcessor();
    double sr = 44100.0;
    int blockSize = 512;
    proc->prepareToPlay(sr, blockSize);
    proc->setPlayConfigDetails(2, 2, sr, blockSize);

    // Generate stereo test signal: L = sine 440Hz, R = sine 880Hz
    int numSamples = blockSize;
    juce::AudioBuffer<float> buffer(2, numSamples);
    juce::AudioBuffer<float> original(2, numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float t = (float)i / (float)sr;
        buffer.setSample(0, i, std::sin(2.0f * 3.14159f * 440.0f * t) * 0.5f);
        buffer.setSample(1, i, std::sin(2.0f * 3.14159f * 880.0f * t) * 0.5f);
    }
    original.makeCopyOf(buffer);

    // --- TEST 1: All defaults (Amount=0) — should be identity ---
    printf("TEST 1: All defaults (Amount=0)\n");
    {
        juce::MidiBuffer midi;
        proc->processBlock(buffer, midi);

        float maxDiff = 0.0f;
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < numSamples; ++i)
                maxDiff = std::max(maxDiff, std::abs(buffer.getSample(ch, i) - original.getSample(ch, i)));

        printf("  Max difference: %.8f\n", maxDiff);
        if (maxDiff < 0.0001f)
            printf("  -> PASS: dry signal passes through unchanged\n\n");
        else
            printf("  -> Signal changed (unexpected at Amount=0)\n\n");
    }

    // --- TEST 2: Amount = 1.0 — should change audio ---
    printf("TEST 2: Amount=1.0 (max)\n");
    {
        // Reset buffer
        for (int i = 0; i < numSamples; ++i)
        {
            float t = (float)i / (float)sr;
            buffer.setSample(0, i, std::sin(2.0f * 3.14159f * 440.0f * t) * 0.5f);
            buffer.setSample(1, i, std::sin(2.0f * 3.14159f * 880.0f * t) * 0.5f);
        }
        original.makeCopyOf(buffer);

        // Set Amount to 1.0
        if (auto* p = proc->apvts.getParameter("amount"))
            p->setValueNotifyingHost(1.0f);

        // Process a few blocks for smoothing to converge
        juce::MidiBuffer midi;
        for (int b = 0; b < 10; ++b)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float t = (float)i / (float)sr;
                buffer.setSample(0, i, std::sin(2.0f * 3.14159f * 440.0f * t) * 0.5f);
                buffer.setSample(1, i, std::sin(2.0f * 3.14159f * 880.0f * t) * 0.5f);
            }
            if (b == 9) original.makeCopyOf(buffer);
            proc->processBlock(buffer, midi);
        }

        float maxDiff = 0.0f;
        float rmsL = 0.0f, rmsR = 0.0f;
        for (int i = 0; i < numSamples; ++i)
        {
            float dL = buffer.getSample(0, i) - original.getSample(0, i);
            float dR = buffer.getSample(1, i) - original.getSample(1, i);
            maxDiff = std::max(maxDiff, std::max(std::abs(dL), std::abs(dR)));
            rmsL += dL * dL;
            rmsR += dR * dR;
        }
        rmsL = std::sqrt(rmsL / numSamples);
        rmsR = std::sqrt(rmsR / numSamples);

        printf("  Max difference: %.6f\n", maxDiff);
        printf("  RMS diff L: %.6f  R: %.6f\n", rmsL, rmsR);
        if (maxDiff > 0.001f)
            printf("  -> PASS: Amount=1.0 CHANGES the audio!\n\n");
        else
            printf("  -> FAIL: no audible change at Amount=1.0\n\n");
    }

    // --- TEST 3: Texture knob at 1.0, Amount=0 ---
    printf("TEST 3: Texture=1.0 (Amount still 1.0)\n");
    {
        if (auto* p = proc->apvts.getParameter("texture"))
            p->setValueNotifyingHost(1.0f);

        juce::MidiBuffer midi;
        for (int b = 0; b < 10; ++b)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float t = (float)i / (float)sr;
                buffer.setSample(0, i, std::sin(2.0f * 3.14159f * 440.0f * t) * 0.5f);
                buffer.setSample(1, i, std::sin(2.0f * 3.14159f * 880.0f * t) * 0.5f);
            }
            if (b == 9) original.makeCopyOf(buffer);
            proc->processBlock(buffer, midi);
        }

        float maxDiff = 0.0f;
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < numSamples; ++i)
                maxDiff = std::max(maxDiff, std::abs(buffer.getSample(ch, i) - original.getSample(ch, i)));

        printf("  Max difference: %.6f\n", maxDiff);
        if (maxDiff > 0.001f)
            printf("  -> PASS: Texture knob CHANGES the audio!\n\n");
        else
            printf("  -> FAIL: no change with Texture=1.0\n\n");
    }

    // --- TEST 4: Air knob ---
    printf("TEST 4: Air=1.0\n");
    {
        if (auto* p = proc->apvts.getParameter("air"))
            p->setValueNotifyingHost(1.0f);

        juce::MidiBuffer midi;
        for (int b = 0; b < 10; ++b)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float t = (float)i / (float)sr;
                buffer.setSample(0, i, std::sin(2.0f * 3.14159f * 440.0f * t) * 0.5f);
                buffer.setSample(1, i, std::sin(2.0f * 3.14159f * 880.0f * t) * 0.5f);
            }
            if (b == 9) original.makeCopyOf(buffer);
            proc->processBlock(buffer, midi);
        }

        float maxDiff = 0.0f;
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < numSamples; ++i)
                maxDiff = std::max(maxDiff, std::abs(buffer.getSample(ch, i) - original.getSample(ch, i)));

        printf("  Max difference: %.6f\n", maxDiff);
        if (maxDiff > 0.001f)
            printf("  -> PASS: Air knob CHANGES the audio!\n\n");
        else
            printf("  -> FAIL: no change with Air=1.0\n\n");
    }

    // --- TEST 5: Profile switching ---
    printf("TEST 5: Profile = Ambient (index 4)\n");
    {
        if (auto* p = proc->apvts.getParameter("profile"))
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(4.0f));

        juce::MidiBuffer midi;
        for (int b = 0; b < 10; ++b)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float t = (float)i / (float)sr;
                buffer.setSample(0, i, std::sin(2.0f * 3.14159f * 440.0f * t) * 0.5f);
                buffer.setSample(1, i, std::sin(2.0f * 3.14159f * 880.0f * t) * 0.5f);
            }
            if (b == 9) original.makeCopyOf(buffer);
            proc->processBlock(buffer, midi);
        }

        float maxDiff = 0.0f;
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < numSamples; ++i)
                maxDiff = std::max(maxDiff, std::abs(buffer.getSample(ch, i) - original.getSample(ch, i)));

        printf("  Max difference: %.6f\n", maxDiff);
        if (maxDiff > 0.001f)
            printf("  -> PASS: Ambient profile active\n\n");
        else
            printf("  -> FAIL: no change\n\n");
    }

    // Cleanup
    proc->releaseResources();
    delete proc;

    printf("=== ALL CHECKS COMPLETE ===\n");
    return 0;
}
