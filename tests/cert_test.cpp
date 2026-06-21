// cert_test.cpp — VST3 Stability Certification Test
// No JUCEApplication. Pure win32 console.
// Tests the processor directly.

#include <windows.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <atomic>
#include <cstring>

// Minimal JUCE includes for the test
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Source/PluginProcessor.h"

// Globals
static int testsPassed = 0;
static int testsFailed = 0;
static int testsTotal = 0;

struct TestResult {
    const char* name;
    bool passed;
    const char* detail;
};

void log(const char* msg) {
    printf("%s\n", msg);
    fflush(stdout);
}

void logf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
}

// SEH-safe wrapper — implemented in seh_wrapper.c (no C++ unwinding conflict)
extern "C" int seh_run(int (*fn)());

//==============================================================================
// Round-trip a buffer through the processor
bool processAudio(SpatialReactorAudioProcessor* proc, int numChannels, int numSamples, double sr, int blockSize) {
    if (blockSize < 1) blockSize = 1;
    int remaining = numSamples;
    while (remaining > 0) {
        int thisBlock = (remaining < blockSize) ? remaining : blockSize;
        juce::AudioBuffer<float> buf(numChannels, thisBlock);
        for (int ch = 0; ch < numChannels; ++ch)
            for (int s = 0; s < thisBlock; ++s)
                buf.setSample(ch, s, sinf((float)s / (float)thisBlock * 6.283f));
        juce::MidiBuffer midi;
        proc->processBlock(buf, midi);
        // Check for NaN/Inf
        for (int ch = 0; ch < numChannels; ++ch)
            for (int s = 0; s < thisBlock; ++s) {
                float v = buf.getSample(ch, s);
                if (isnan(v) || isinf(v)) return false;
            }
        remaining -= thisBlock;
    }
    return true;
}

//==============================================================================
// Tests — use plain C functions with SEH wrappers

int testConstructorDestroy1000_impl() {
    for (int i = 0; i < 1000; ++i) {
        auto* proc = new SpatialReactorAudioProcessor();
        delete proc;
    }
    return 0;
}
TestResult testConstructorDestroy1000() {
    log("  [1/10] Constructor/destructor 1000x...");
    int r = seh_run(testConstructorDestroy1000_impl);
    return {"Constructor/destructor 1000x", r == 0,
            r == 0 ? "1000 cycles, no crash" : "ACCESS VIOLATION"};
}

int testEditor500_impl() {
    for (int i = 0; i < 500; ++i) {
        auto* proc = new SpatialReactorAudioProcessor();
        auto* editor = proc->createEditor();
        if (editor) {
            editor->setSize(700, 960);
            delete editor;
        }
        delete proc;
        if (i == 0) log("    iteration 0 OK");
        if (i == 1) log("    iteration 1 OK");
        if (i == 2) log("    iteration 2 OK");
        if (i == 4) log("    iteration 4 OK");
        if (i == 9) log("    iteration 9 OK");
    }
    return 0;
}
TestResult testEditor500() {
    log("  [2/10] Editor create/destroy 500x...");
    int r = seh_run(testEditor500_impl);
    if (r == 0) return {"Editor create/destroy 500x", true, "500 cycles, no crash"};
    return {"Editor create/destroy 500x", false, "ACCESS VIOLATION"};
}

//==============================================================================
// Test 3: 100000 processBlock calls
int testProcess100000_impl() {
    auto* proc = new SpatialReactorAudioProcessor();
    proc->prepareToPlay(44100.0, 512);
    for (int i = 0; i < 100000; ++i) {
        if (!processAudio(proc, 2, 128, 44100.0, 64)) {
            delete proc;
            return -2; // NaN or Inf
        }
        if (i % 10000 == 0) logf("    %d/100000\n", i);
    }
    delete proc;
    return 0;
}
TestResult testProcess100000() {
    log("  [3/10] 100000 processBlock calls...");
    int r = seh_run(testProcess100000_impl);
    const char* detail = "100000 blocks, no crash";
    if (r == -1) detail = "ACCESS VIOLATION";
    if (r == -2) detail = "NaN output detected";
    return {"100000 processBlock calls", r == 0, detail};
}

//==============================================================================
// Test 4: Parameter stress
int testParameterStress_impl() {
    auto* proc = new SpatialReactorAudioProcessor();
    proc->prepareToPlay(44100.0, 512);
    auto& params = proc->apvts;
    for (int i = 0; i < 5000; ++i) {
        *params.getRawParameterValue("amount") = (float)rand() / (float)RAND_MAX;
        *params.getRawParameterValue("density") = (float)rand() / (float)RAND_MAX;
        *params.getRawParameterValue("texture") = (float)rand() / (float)RAND_MAX;
        *params.getRawParameterValue("air") = (float)rand() / (float)RAND_MAX;
        *params.getRawParameterValue("trustMode") = (rand() % 2) ? 1.0f : 0.0f;
        *params.getRawParameterValue("profile") = (float)(rand() % 5);
        if (i % 100 == 0) {
            if (!processAudio(proc, 2, 256, 44100.0, 64)) {
                delete proc;
                return -2;
            }
        }
    }
    delete proc;
    return 0;
}
TestResult testParameterStress() {
    log("  [4/10] Parameter stress...");
    int r = seh_run(testParameterStress_impl);
    const char* detail = "5000 param changes, no crash";
    if (r == -1) detail = "ACCESS VIOLATION during param set";
    if (r == -2) detail = "CRASH during audio after param change";
    return {"Parameter stress", r == 0, detail};
}

//==============================================================================
// Test 5: Buffer sizes
TestResult testBufferSizes() {
    log("  [5/10] Buffer sizes...");
    int sizes[] = {1, 16, 32, 64, 128, 256, 512, 1024, 2048};
    for (int si = 0; si < 9; ++si) {
        auto* proc = new SpatialReactorAudioProcessor();
        proc->prepareToPlay(44100.0, sizes[si]);
        if (!processAudio(proc, 2, sizes[si] * 4, 44100.0, sizes[si])) {
            delete proc;
            logf("    FAIL at block size %d\n", sizes[si]);
            return {"Buffer sizes", false, "CRASH at block size"};
        }
        delete proc;
    }
    return {"Buffer sizes", true, "9 block sizes, no crash"};
}

//==============================================================================
// Test 6: Sample rates
TestResult testSampleRates() {
    log("  [6/10] Sample rates...");
    double rates[] = {44100.0, 48000.0, 88200.0, 96000.0, 192000.0};
    for (int ri = 0; ri < 5; ++ri) {
        auto* proc = new SpatialReactorAudioProcessor();
        proc->prepareToPlay(rates[ri], 512);
        if (!processAudio(proc, 2, 4096, rates[ri], 128)) {
            delete proc;
            logf("    FAIL at rate %.0f\n", rates[ri]);
            return {"Sample rates", false, "CRASH at sample rate"};
        }
        delete proc;
    }
    return {"Sample rates", true, "5 sample rates, no crash"};
}

//==============================================================================
// Test 7: Bus layout
TestResult testBusLayout() {
    log("  [7/10] Bus layout (mono/stereo)...");
    auto* proc = new SpatialReactorAudioProcessor();
    proc->prepareToPlay(44100.0, 512);
    
    // Stereo
    if (!processAudio(proc, 2, 1024, 44100.0, 128)) {
        delete proc; return {"Bus layout", false, "CRASH stereo"};
    }
    // Mono  
    if (!processAudio(proc, 1, 1024, 44100.0, 128)) {
        delete proc; return {"Bus layout", false, "CRASH mono"};
    }
    // Switch back to stereo
    if (!processAudio(proc, 2, 1024, 44100.0, 128)) {
        delete proc; return {"Bus layout", false, "CRASH stereo after mono"};
    }
    delete proc;
    return {"Bus layout", true, "Stereo→Mono→Stereo, no crash"};
}

//==============================================================================
// Test 8: State save/load 1000x
int testStateCycle_impl() {
    auto* proc = new SpatialReactorAudioProcessor();
    proc->prepareToPlay(44100.0, 512);
    for (int i = 0; i < 1000; ++i) {
        juce::MemoryBlock block;
        proc->getStateInformation(block);
        proc->setStateInformation(block.getData(), (int)block.getSize());
        if (i % 100 == 0) {
            if (!processAudio(proc, 2, 128, 44100.0, 64)) {
                delete proc;
                return -2;
            }
        }
    }
    delete proc;
    return 0;
}
TestResult testStateCycle() {
    log("  [8/10] State save/load 1000x...");
    int r = seh_run(testStateCycle_impl);
    const char* detail = "1000 cycles, no crash";
    if (r == -1) detail = "ACCESS VIOLATION during state cycle";
    if (r == -2) detail = "CRASH after state restore";
    return {"State save/load 1000x", r == 0, detail};
}

//==============================================================================
//==============================================================================
// Test 9: Editor destruction during automation
int testDestructionDuringAutomation_impl() {
    for (int i = 0; i < 100; ++i) {
        auto* proc = new SpatialReactorAudioProcessor();
        proc->prepareToPlay(44100.0, 512);
        auto* editor = proc->createEditor();
        if (editor) {
            editor->setSize(700, 960);
            for (int p = 0; p < 10; ++p) {
                *proc->apvts.getRawParameterValue("amount") = (float)p / 10.0f;
            }
            delete editor;
        }
        if (!processAudio(proc, 2, 128, 44100.0, 64)) {
            delete proc;
            return -2;
        }
        delete proc;
    }
    return 0;
}
TestResult testDestructionDuringAutomation() {
    log("  [9/10] Destruction during automation...");
    int r = seh_run(testDestructionDuringAutomation_impl);
    const char* detail = "100 cycles, no crash";
    if (r == -1) detail = "ACCESS VIOLATION during destruction";
    if (r == -2) detail = "CRASH after editor destroy";
    return {"Destruction during automation", r == 0, detail};
}

//==============================================================================
// Test 10: Oversampling switching
TestResult testOversamplingSwitch() {
    log("  [10/10] Oversampling switching...");
    auto* proc = new SpatialReactorAudioProcessor();
    proc->prepareToPlay(44100.0, 512);
    auto& params = proc->apvts;
    
    for (int i = 0; i < 500; ++i) {
        // Cycle through Silk(0)->Dust(0.5)->Rust(1.0)
        float t = (float)(i % 3) / 2.0f;
        *params.getRawParameterValue("texture") = t;
        if (!processAudio(proc, 2, 256, 44100.0, 64)) {
            delete proc;
            return {"Oversampling switching", false, "CRASH during oversampling switch"};
        }
    }
    delete proc;
    return {"Oversampling switching", true, "500 switches, no crash"};
}

//==============================================================================
int main()
{
    log("========================================");
    log(" VST3 STABILITY CERTIFICATION TEST SUITE");
    log("========================================");
    log("");

    // Initialize JUCE core (minimal, no JUCEApplication)
    juce::MessageManager::getInstance();

    TestResult tests[] = {
        testConstructorDestroy1000(),
        testEditor500(),
        testProcess100000(),
        testParameterStress(),
        testBufferSizes(),
        testSampleRates(),
        testBusLayout(),
        testStateCycle(),
        testDestructionDuringAutomation(),
        testOversamplingSwitch()
    };

    log("");
    log("========================================");
    log(" RESULTS");
    log("========================================");

    int passed = 0, failed = 0;
    for (int i = 0; i < 10; ++i) {
        const char* status = tests[i].passed ? "PASS" : "FAIL";
        printf(" [%s] %s\n", status, tests[i].name);
        if (!tests[i].passed) {
            printf("       %s\n", tests[i].detail);
            failed++;
        } else {
            passed++;
        }
    }

    log("");
    log("========================================");
    logf(" %d PASSED, %d FAILED\n", passed, failed);

    if (failed > 0) {
        log("");
        log(" CERTIFICATION: FAILED");
        log(" The following tests crashed:");
        for (int i = 0; i < 10; ++i) {
            if (!tests[i].passed) {
                printf("   - %s: %s\n", tests[i].name, tests[i].detail);
            }
        }
    } else {
        log(" ALL TESTS PASSED");
        log(" CERTIFICATION: PASSED");
    }
    log("========================================");

    return failed > 0 ? 1 : 0;
}
