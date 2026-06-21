#pragma once

#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"

//==============================================================================
/**
    Hostile DAW simulation test suite.
    Tests the plugin under adversarial conditions a DAW might create.
    Every test returns true = PASS, false = FAIL with crash detail.
*/
class HostileDAWTest
{
public:
    struct Result
    {
        juce::String name;
        bool passed;
        juce::String detail;
        int crashProbability; // 0-100%
    };

    // Run all tests and return results
    static juce::Array<Result> runAll();

private:
    // Individual tests
    static Result testProcessBeforePrepare();
    static Result testPrepareToPlayTwice();
    static Result testReleaseResourcesRepeatedly();
    static Result testEditorCreateDestroy1000();
    static Result testEditorDuringAudio();
    static Result testRandomizeParams10Min();
    static Result testAudioRateAutomation();
    static Result testOversamplingEveryBuffer();
    static Result testBusLayoutSwitch();
    static Result testZeroLengthBuffer();
    static Result testOneSampleBuffer();
    static Result testLargeBuffer();
    static Result testNaNFed();
    static Result testMultiSampleRate();
    static Result testMultiBlockSize();
    static Result test100Instances();
    static Result testRandomDestroy();
    static Result testAPVTSParameterStress();
    static Result testMemoryCorruption();
    static Result testRaceCondition();

    // Helpers
    static std::unique_ptr<SpatialReactorAudioProcessor> createProcessor();
    static void processBuffer (juce::AudioProcessor& proc, int numChannels, int numSamples, double sampleRate, int blockSize);
    static bool isBufferValid (const juce::AudioBuffer<float>& buffer);
    static float nanValue();
    static float infValue();
    static float denormalValue();
};
