#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

// Define plugin macros for standalone build
#ifndef JucePlugin_Name
#define JucePlugin_Name "Spatial Reactor"
#endif
#ifndef JucePlugin_IsMidiEffect
#define JucePlugin_IsMidiEffect 0
#endif
#ifndef JucePlugin_WantsMidiInput
#define JucePlugin_WantsMidiInput 0
#endif
#ifndef JucePlugin_ProducesMidiOutput
#define JucePlugin_ProducesMidiOutput 0
#endif
#ifndef JucePlugin_IsSynth
#define JucePlugin_IsSynth 0
#endif
#include "HostileDAWTest.h"

//==============================================================================
/**
    Test runner main — simulates a hostile DAW environment.
*/
class TestRunner : public juce::JUCEApplication
{
public:
    TestRunner() {}

    const juce::String getApplicationName() override { return "Spatial Reactor Test Suite"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    void initialise (const juce::String&) override
    {
        juce::Logger::outputDebugString ("");
        juce::Logger::outputDebugString ("========================================");
        juce::Logger::outputDebugString (" SPATIAL REACTOR — HOSTILE DAW TEST SUITE");
        juce::Logger::outputDebugString ("========================================");
        juce::Logger::outputDebugString ("");

        auto results = HostileDAWTest::runAll();

        int passed = 0, failed = 0;
        int totalCrashProb = 0;

        juce::Logger::outputDebugString (juce::String::formatted (" %3d tests executed\n", results.size()));
        juce::Logger::outputDebugString ("");

        for (auto& r : results)
        {
            auto status = r.passed ? "PASS" : "FAIL";
            auto icon   = r.passed ? "[✓]" : "[✗]";

            juce::Logger::outputDebugString (
                juce::String::formatted (" %s %s", icon, r.name));

            if (!r.passed)
            {
                juce::Logger::outputDebugString (
                    juce::String::formatted ("     Detail: %s", r.detail));
                juce::Logger::outputDebugString (
                    juce::String::formatted ("     DAW crash probability: %d%%", r.crashProbability));
            }

            if (r.passed) passed++; else failed++;
            totalCrashProb += r.crashProbability;
        }

        int avgCrashProb = results.size() > 0 ? totalCrashProb / results.size() : 0;

        juce::Logger::outputDebugString ("");
        juce::Logger::outputDebugString ("========================================");
        juce::Logger::outputDebugString (juce::String::formatted (" %d PASSED, %d FAILED", passed, failed));
        juce::Logger::outputDebugString (juce::String::formatted (" Average DAW crash probability: %d%%", avgCrashProb));

        if (failed > 0)
        {
            juce::Logger::outputDebugString ("");
            juce::Logger::outputDebugString (" CRASH REPORT — UNSTABLE");
            juce::Logger::outputDebugString (" Do not ship this plugin.");
            for (auto& r : results)
            {
                if (!r.passed)
                {
                    juce::Logger::outputDebugString (
                        juce::String::formatted ("   FAIL: %s", r.name));
                    juce::Logger::outputDebugString (
                        juce::String::formatted ("         %s", r.detail));
                }
            }
        }
        else
        {
            juce::Logger::outputDebugString ("");
            juce::Logger::outputDebugString (" REPORT — STABLE");
            juce::Logger::outputDebugString (" Plugin passes all hostile DAW simulations.");
        }

        juce::Logger::outputDebugString ("========================================");
        juce::Logger::outputDebugString ("");

        // Exit
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

    void shutdown() override {}
};

START_JUCE_APPLICATION (TestRunner)
