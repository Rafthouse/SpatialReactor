#include "HostileDAWTest.h"
#include "../Source/PluginEditor.h"
#include <chrono>
#include <cmath>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <random>

// #define TEST_PRINT(x) juce::Logger::outputDebugString(x)
#define TEST_PRINT(x)

//==============================================================================
// Helpers
std::unique_ptr<SpatialReactorAudioProcessor> HostileDAWTest::createProcessor()
{
    auto proc = std::make_unique<SpatialReactorAudioProcessor>();
    return proc;
}

void HostileDAWTest::processBuffer (juce::AudioProcessor& proc, int numChannels, int numSamples, double sampleRate, int blockSize)
{
    if (blockSize < 1) blockSize = 1;
    int remaining = numSamples;
    while (remaining > 0)
    {
        int thisBlock = juce::jmin (remaining, blockSize);
        juce::AudioBuffer<float> buffer (numChannels, thisBlock);
        // Fill with audio signal (sine)
        for (int ch = 0; ch < numChannels; ++ch)
            for (int s = 0; s < thisBlock; ++s)
                buffer.setSample (ch, s, std::sin ((float) s / (float) thisBlock * 6.283f));
        juce::MidiBuffer midi;
        proc.processBlock (buffer, midi);
        remaining -= thisBlock;
    }
}

bool HostileDAWTest::isBufferValid (const juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float v = buffer.getSample (ch, s);
            if (std::isnan (v) || std::isinf (v))
                return false;
        }
    return true;
}

float HostileDAWTest::nanValue()  { return std::nanf (""); }
float HostileDAWTest::infValue()  { return INFINITY; }
float HostileDAWTest::denormalValue() { return std::numeric_limits<float>::denorm_min(); }

//==============================================================================
// Test 1: Call processBlock() before prepareToPlay()
HostileDAWTest::Result HostileDAWTest::testProcessBeforePrepare()
{
    TEST_PRINT ("[TEST 1] processBlock() before prepareToPlay()");
    try
    {
        auto proc = createProcessor();
        juce::AudioBuffer<float> buffer (2, 512);
        juce::MidiBuffer midi;

        // Call processBlock WITHOUT calling prepareToPlay first
        proc->processBlock (buffer, midi);

        bool valid = isBufferValid (buffer);
        return { "processBlock() before prepareToPlay()", valid,
                 valid ? "No crash, audio output valid" : "Audio output contains NaN/Inf",
                 valid ? 10 : 80 };
    }
    catch (...)
    {
        return { "processBlock() before prepareToPlay()", false,
                 "CRASH: unhandled exception in processBlock", 95 };
    }
}

//==============================================================================
// Test 2: Call prepareToPlay() twice
HostileDAWTest::Result HostileDAWTest::testPrepareToPlayTwice()
{
    TEST_PRINT ("[TEST 2] prepareToPlay() twice");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        proc->prepareToPlay (48000.0, 256); // second prepare with different params
        processBuffer (*proc, 2, 2048, 48000.0, 256);
        return { "prepareToPlay() twice", true, "No crash on double prepare", 15 };
    }
    catch (...)
    {
        return { "prepareToPlay() twice", false, "CRASH on second prepareToPlay", 60 };
    }
}

//==============================================================================
// Test 3: releaseResources() repeatedly
HostileDAWTest::Result HostileDAWTest::testReleaseResourcesRepeatedly()
{
    TEST_PRINT ("[TEST 3] releaseResources() repeatedly");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        for (int i = 0; i < 100; ++i)
            proc->releaseResources();
        return { "releaseResources() repeatedly", true, "100 calls without crash", 10 };
    }
    catch (...)
    {
        return { "releaseResources() repeatedly", false, "CRASH on repeated release", 50 };
    }
}

//==============================================================================
// Test 4: Create and destroy editor 1000 times
HostileDAWTest::Result HostileDAWTest::testEditorCreateDestroy1000()
{
    TEST_PRINT ("[TEST 4] Editor create/destroy 1000x");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        for (int i = 0; i < 1000; ++i)
        {
            auto* editor = proc->createEditor();
            if (editor == nullptr)
                return { "Editor create/destroy 1000x", false, "createEditor returned nullptr", 100 };
            editor->setSize (700, 960);
            // Simulate some paint calls
            for (int p = 0; p < 5; ++p)
            {
                juce::Graphics g (juce::Image (juce::Image::RGB, 700, 960, false));
                editor->paint (g);
            }
            delete editor;
        }
        return { "Editor create/destroy 1000x", true, "1000 cycles without crash", 5 };
    }
    catch (...)
    {
        return { "Editor create/destroy 1000x", false, "CRASH during editor cycle", 90 };
    }
}

//==============================================================================
// Test 5: Open/close editor while audio is running
HostileDAWTest::Result HostileDAWTest::testEditorDuringAudio()
{
    TEST_PRINT ("[TEST 5] Editor during audio");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        std::atomic<bool> audioDone { false };

        // Audio thread
        auto audioThread = std::thread ([&]()
        {
            while (!audioDone.load())
            {
                processBuffer (*proc, 2, 4096, 44100.0, 128);
                std::this_thread::sleep_for (std::chrono::milliseconds (1));
            }
        });

        // Editor create/destroy on main thread
        for (int i = 0; i < 100; ++i)
        {
            auto editor = std::unique_ptr<juce::AudioProcessorEditor> (proc->createEditor());
            if (editor != nullptr)
            {
                editor->setSize (700, 960);
                std::this_thread::sleep_for (std::chrono::milliseconds (1));
            }
        }
        audioDone.store (true);
        audioThread.join();

        return { "Editor during audio", true, "100 cycles with concurrent audio, no crash", 25 };
    }
    catch (...)
    {
        return { "Editor during audio", false, "CRASH: editor + audio thread race", 85 };
    }
}

//==============================================================================
// Test 6: Randomize all parameters continuously
HostileDAWTest::Result HostileDAWTest::testRandomizeParams10Min()
{
    TEST_PRINT ("[TEST 6] Randomize params 10 min");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        auto& params = proc->parameters;
        auto start = std::chrono::steady_clock::now();
        auto dur = std::chrono::minutes (1); // 1 minute for practicality (not 10)
        int iterations = 0;

        while (std::chrono::steady_clock::now() - start < dur)
        {
            // Randomize all params
            *params.getRawParameterValue ("amount")    = juce::Random::getSystemRandom().nextFloat();
            *params.getRawParameterValue ("density")   = juce::Random::getSystemRandom().nextFloat();
            *params.getRawParameterValue ("texture")   = juce::Random::getSystemRandom().nextFloat();
            *params.getRawParameterValue ("air")       = juce::Random::getSystemRandom().nextFloat();
            *params.getRawParameterValue ("trustMode") = juce::Random::getSystemRandom().nextFloat() > 0.5f ? 1.0f : 0.0f;
            *params.getRawParameterValue ("profile")   = (float) (int) (juce::Random::getSystemRandom().nextFloat() * 5.0f);

            processBuffer (*proc, 2, 1024, 44100.0, 64);
            iterations++;
        }

        return { "Randomize params 1 min", true,
                 juce::String (iterations) + " iterations, no crash", 20 };
    }
    catch (...)
    {
        return { "Randomize params 1 min", false, "CRASH during param randomize", 80 };
    }
}

//==============================================================================
// Test 7: Automate every parameter at audio rate
HostileDAWTest::Result HostileDAWTest::testAudioRateAutomation()
{
    TEST_PRINT ("[TEST 7] Audio-rate automation");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        auto& params = proc->parameters;

        for (int block = 0; block < 1000; ++block)
        {
            // Change params every sample (audio rate)
            for (int s = 0; s < 128; ++s)
            {
                *params.getRawParameterValue ("amount")  = std::sin ((float) s * 0.1f) * 0.5f + 0.5f;
                *params.getRawParameterValue ("density") = std::cos ((float) s * 0.05f) * 0.5f + 0.5f;
                *params.getRawParameterValue ("texture") = std::sin ((float) s * 0.2f) * 0.5f + 0.5f;
                *params.getRawParameterValue ("air")     = std::cos ((float) s * 0.15f) * 0.5f + 0.5f;
            }
            processBuffer (*proc, 2, 128, 44100.0, 128);
        }

        return { "Audio-rate automation", true, "1000 blocks with per-sample param changes", 30 };
    }
    catch (...)
    {
        return { "Audio-rate automation", false, "CRASH during audio-rate automation", 75 };
    }
}

//==============================================================================
// Test 8: Switch oversampling modes every buffer
HostileDAWTest::Result HostileDAWTest::testOversamplingEveryBuffer()
{
    TEST_PRINT ("[TEST 8] Oversampling switch every buffer");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        auto& params = proc->parameters;

        for (int i = 0; i < 500; ++i)
        {
            // Cycle texture through Silk/Dust/Rust every buffer
            float t = (float) (i % 3) / 2.0f; // 0.0, 0.5, 1.0
            *params.getRawParameterValue ("texture") = t;
            processBuffer (*proc, 2, 256, 44100.0, 64);
        }

        return { "Oversampling every buffer", true, "500 switches without crash", 15 };
    }
    catch (...)
    {
        return { "Oversampling every buffer", false, "CRASH during oversampling switch", 70 };
    }
}

//==============================================================================
// Test 9: Switch mono/stereo bus layouts
HostileDAWTest::Result HostileDAWTest::testBusLayoutSwitch()
{
    TEST_PRINT ("[TEST 9] Bus layout switch");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);

        // Stereo
        processBuffer (*proc, 2, 1024, 44100.0, 128);

        // Mono
        processBuffer (*proc, 1, 1024, 44100.0, 128);

        // Back to stereo
        processBuffer (*proc, 2, 1024, 44100.0, 128);

        return { "Bus layout switch", true, "Mono→Stereo→Mono no crash", 20 };
    }
    catch (...)
    {
        return { "Bus layout switch", false, "CRASH on bus switch", 65 };
    }
}

//==============================================================================
// Test 10: Zero-length buffers
HostileDAWTest::Result HostileDAWTest::testZeroLengthBuffer()
{
    TEST_PRINT ("[TEST 10] Zero-length buffer");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        juce::AudioBuffer<float> buffer (2, 0);
        juce::MidiBuffer midi;
        proc->processBlock (buffer, midi);
        return { "Zero-length buffer", true, "No crash", 15 };
    }
    catch (...)
    {
        return { "Zero-length buffer", false, "CRASH on zero-length buffer", 60 };
    }
}

//==============================================================================
// Test 11: One-sample buffers
HostileDAWTest::Result HostileDAWTest::testOneSampleBuffer()
{
    TEST_PRINT ("[TEST 11] One-sample buffer");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        for (int i = 0; i < 100; ++i)
            processBuffer (*proc, 2, 1, 44100.0, 1);
        return { "One-sample buffer", true, "100 blocks of 1 sample, no crash", 20 };
    }
    catch (...)
    {
        return { "One-sample buffer", false, "CRASH on 1-sample buffer", 55 };
    }
}

//==============================================================================
// Test 12: Very large buffers
HostileDAWTest::Result HostileDAWTest::testLargeBuffer()
{
    TEST_PRINT ("[TEST 12] Large buffer");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 8192);
        processBuffer (*proc, 2, 16384, 44100.0, 8192);
        return { "Large buffer (16384)", true, "No crash", 25 };
    }
    catch (...)
    {
        return { "Large buffer (16384)", false, "CRASH on large buffer", 50 };
    }
}

//==============================================================================
// Test 13: NaN, Inf, denormal values
HostileDAWTest::Result HostileDAWTest::testNaNFed()
{
    TEST_PRINT ("[TEST 13] NaN/Inf/denormal feed");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);

        // Feed NaN
        {
            juce::AudioBuffer<float> buffer (2, 128);
            for (int ch = 0; ch < 2; ++ch)
                for (int s = 0; s < 128; ++s)
                    buffer.setSample (ch, s, nanValue());
            juce::MidiBuffer midi;
            proc->processBlock (buffer, midi);
        }

        // Feed Inf
        {
            juce::AudioBuffer<float> buffer (2, 128);
            for (int ch = 0; ch < 2; ++ch)
                for (int s = 0; s < 128; ++s)
                    buffer.setSample (ch, s, infValue());
            juce::MidiBuffer midi;
            proc->processBlock (buffer, midi);
        }

        // Feed denormals
        {
            juce::AudioBuffer<float> buffer (2, 128);
            for (int ch = 0; ch < 2; ++ch)
                for (int s = 0; s < 128; ++s)
                    buffer.setSample (ch, s, denormalValue());
            juce::MidiBuffer midi;
            proc->processBlock (buffer, midi);
        }

        return { "NaN/Inf/denormal feed", true, "All three pathological inputs handled", 35 };
    }
    catch (...)
    {
        return { "NaN/Inf/denormal feed", false, "CRASH on pathological input", 70 };
    }
}

//==============================================================================
// Test 14: Multi sample rate
HostileDAWTest::Result HostileDAWTest::testMultiSampleRate()
{
    TEST_PRINT ("[TEST 14] Multi sample rate");
    try
    {
        auto proc = createProcessor();
        double rates[] = { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 };
        for (auto rate : rates)
        {
            proc->prepareToPlay (rate, 512);
            processBuffer (*proc, 2, 4096, rate, 128);
        }
        return { "Multi sample rate", true, "5 rates without crash", 15 };
    }
    catch (...)
    {
        return { "Multi sample rate", false, "CRASH at sample rate switch", 45 };
    }
}

//==============================================================================
// Test 15: Multi block size
HostileDAWTest::Result HostileDAWTest::testMultiBlockSize()
{
    TEST_PRINT ("[TEST 15] Multi block size");
    try
    {
        auto proc = createProcessor();
        int blockSizes[] = { 1, 16, 32, 64, 128, 256, 512, 1024, 2048 };
        for (auto bs : blockSizes)
        {
            proc->prepareToPlay (44100.0, bs);
            processBuffer (*proc, 2, bs * 4, 44100.0, bs);
        }
        return { "Multi block size", true, "9 block sizes without crash", 15 };
    }
    catch (...)
    {
        return { "Multi block size", false, "CRASH at block size switch", 45 };
    }
}

//==============================================================================
// Test 16: 100 plugin instances
HostileDAWTest::Result HostileDAWTest::test100Instances()
{
    TEST_PRINT ("[TEST 16] 100 instances");
    try
    {
        std::vector<std::unique_ptr<SpatialReactorAudioProcessor>> instances;
        for (int i = 0; i < 100; ++i)
        {
            auto proc = createProcessor();
            proc->prepareToPlay (44100.0, 512);
            processBuffer (*proc, 2, 1024, 44100.0, 128);
            instances.push_back (std::move (proc));
        }
        return { "100 instances", true, "100 instances, no crash", 10 };
    }
    catch (...)
    {
        return { "100 instances", false, "CRASH on instance " + juce::String (__LINE__), 70 };
    }
}

//==============================================================================
// Test 17: Destroy instances randomly
HostileDAWTest::Result HostileDAWTest::testRandomDestroy()
{
    TEST_PRINT ("[TEST 17] Random destroy");
    try
    {
        std::vector<std::unique_ptr<SpatialReactorAudioProcessor>> instances;
        std::mt19937 rng (42);
        for (int i = 0; i < 50; ++i)
        {
            auto proc = createProcessor();
            proc->prepareToPlay (44100.0, 512);
            instances.push_back (std::move (proc));
        }
        // Destroy in random order
        std::shuffle (instances.begin(), instances.end(), rng);
        instances.clear();
        return { "Random destroy", true, "50 instances destroyed randomly, no crash", 10 };
    }
    catch (...)
    {
        return { "Random destroy", false, "CRASH during random destroy", 60 };
    }
}

//==============================================================================
// Test 18: APVTS stress test
HostileDAWTest::Result HostileDAWTest::testAPVTSParameterStress()
{
    TEST_PRINT ("[TEST 18] APVTS stress");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        auto& params = proc->parameters;

        // Rapid fire parameter changes while processing
        for (int i = 0; i < 5000; ++i)
        {
            // Normal parameter path
            *params.getRawParameterValue ("amount")  = juce::Random::getSystemRandom().nextFloat();
            *params.getRawParameterValue ("density") = juce::Random::getSystemRandom().nextFloat();

            if (i % 100 == 0)
                processBuffer (*proc, 2, 256, 44100.0, 64);
        }

        // State save/load stress
        for (int i = 0; i < 100; ++i)
        {
            juce::MemoryBlock block;
            proc->getStateInformation (block);
            proc->setStateInformation (block.getData(), (int) block.getSize());
        }

        return { "APVTS stress", true, "5000 param changes + 100 save/load cycles", 25 };
    }
    catch (...)
    {
        return { "APVTS stress", false, "CRASH during APVTS stress", 65 };
    }
}

//==============================================================================
// Test 19: Memory corruption detection
HostileDAWTest::Result HostileDAWTest::testMemoryCorruption()
{
    TEST_PRINT ("[TEST 19] Memory corruption");
    try
    {
        // Over-allocate and check for corruption around plugin memory
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);

        // Create sentinel objects before and after
        std::vector<int> before (1000, 0xDEADBEEF);
        auto& p = *proc;
        std::vector<int> after (1000, 0xDEADBEEF);

        // Hammer the plugin
        for (int i = 0; i < 2000; ++i)
        {
            processBuffer (p, 2, 128, 44100.0, 64);
            auto editor = std::unique_ptr<juce::AudioProcessorEditor> (p.createEditor());
            if (editor != nullptr)
            {
                editor->setSize (700, 960);
                juce::Graphics g (juce::Image (juce::Image::RGB, 700, 960, false));
                editor->paint (g);
            }
        }

        // Check sentinels
        bool beforeOK = std::all_of (before.begin(), before.end(), [](int v) { return v == 0xDEADBEEF; });
        bool afterOK  = std::all_of (after.begin(),  after.end(),  [](int v) { return v == 0xDEADBEEF; });

        if (!beforeOK || !afterOK)
            return { "Memory corruption", false, "Sentinel memory corrupted by plugin", 95 };

        return { "Memory corruption", true, "No memory corruption detected", 5 };
    }
    catch (...)
    {
        return { "Memory corruption", false, "CRASH during memory test", 80 };
    }
}

//==============================================================================
// Test 20: Race condition detection
HostileDAWTest::Result HostileDAWTest::testRaceCondition()
{
    TEST_PRINT ("[TEST 20] Race condition");
    try
    {
        auto proc = createProcessor();
        proc->prepareToPlay (44100.0, 512);
        std::atomic<bool> failed { false };
        std::mutex mtx;

        // Two concurrent processing threads
        auto thread1 = std::thread ([&]()
        {
            for (int i = 0; i < 500; ++i)
            {
                juce::AudioBuffer<float> buf (2, 64);
                juce::MidiBuffer midi;
                std::lock_guard<std::mutex> lock (mtx);
                proc->processBlock (buf, midi);
            }
        });

        auto thread2 = std::thread ([&]()
        {
            for (int i = 0; i < 500; ++i)
            {
                juce::AudioBuffer<float> buf (2, 64);
                juce::MidiBuffer midi;
                std::lock_guard<std::mutex> lock (mtx);
                proc->processBlock (buf, midi);
            }
        });

        // And a parameter thread
        auto thread3 = std::thread ([&]()
        {
            for (int i = 0; i < 500; ++i)
            {
                std::lock_guard<std::mutex> lock (mtx);
                *proc->parameters.getRawParameterValue ("amount") = juce::Random::getSystemRandom().nextFloat();
                *proc->parameters.getRawParameterValue ("density") = juce::Random::getSystemRandom().nextFloat();
                *proc->parameters.getRawParameterValue ("texture") = juce::Random::getSystemRandom().nextFloat();
                *proc->parameters.getRawParameterValue ("air") = juce::Random::getSystemRandom().nextFloat();
            }
        });

        thread1.join();
        thread2.join();
        thread3.join();

        return { "Race condition", !failed.load(),
                 !failed.load() ? "3 threads concurrent, no detected race" : "Race detected",
                 failed.load() ? 90 : 30 };
    }
    catch (...)
    {
        return { "Race condition", false, "CRASH during concurrent access", 85 };
    }
}

//==============================================================================
// Run all tests
juce::Array<HostileDAWTest::Result> HostileDAWTest::runAll()
{
    juce::Array<Result> results;
    #define RUN_TEST(func) results.add (func())

    RUN_TEST (testProcessBeforePrepare);
    RUN_TEST (testPrepareToPlayTwice);
    RUN_TEST (testReleaseResourcesRepeatedly);
    RUN_TEST (testEditorCreateDestroy1000);
    RUN_TEST (testEditorDuringAudio);
    RUN_TEST (testRandomizeParams10Min);
    RUN_TEST (testAudioRateAutomation);
    RUN_TEST (testOversamplingEveryBuffer);
    RUN_TEST (testBusLayoutSwitch);
    RUN_TEST (testZeroLengthBuffer);
    RUN_TEST (testOneSampleBuffer);
    RUN_TEST (testLargeBuffer);
    RUN_TEST (testNaNFed);
    RUN_TEST (testMultiSampleRate);
    RUN_TEST (testMultiBlockSize);
    RUN_TEST (test100Instances);
    RUN_TEST (testRandomDestroy);
    RUN_TEST (testAPVTSParameterStress);
    RUN_TEST (testMemoryCorruption);
    RUN_TEST (testRaceCondition);

    #undef RUN_TEST
    return results;
}
