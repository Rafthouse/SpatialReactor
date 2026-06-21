#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Source/PluginProcessor.h"

class MinimalTest : public juce::JUCEApplication
{
public:
    MinimalTest() {}

    const juce::String getApplicationName() override { return "MinTest"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    void initialise (const juce::String&) override
    {
        juce::Logger::outputDebugString ("=== MINIMAL TEST ===");

        // Step 1: constructor
        juce::Logger::outputDebugString ("Creating processor...");
        auto proc = std::make_unique<SpatialReactorAudioProcessor>();
        juce::Logger::outputDebugString ("Processor created OK");

        // Step 2: prepareToPlay
        juce::Logger::outputDebugString ("Calling prepareToPlay...");
        proc->prepareToPlay (44100.0, 512);
        juce::Logger::outputDebugString ("prepareToPlay OK");

        // Step 3: processBlock
        juce::Logger::outputDebugString ("Calling processBlock...");
        juce::AudioBuffer<float> buf (2, 128);
        for (int i = 0; i < 128; ++i)
        {
            buf.setSample (0, i, std::sin ((float) i / 128.0f * 6.283f));
            buf.setSample (1, i, std::cos ((float) i / 128.0f * 6.283f));
        }
        juce::MidiBuffer midi;
        proc->processBlock (buf, midi);
        juce::Logger::outputDebugString ("processBlock OK");

        // Step 4: check output
        bool hasNan = false;
        for (int ch = 0; ch < 2; ++ch)
            for (int s = 0; s < 128; ++s)
                if (std::isnan (buf.getSample (ch, s)) || std::isinf (buf.getSample (ch, s)))
                    hasNan = true;

        juce::Logger::outputDebugString (hasNan ? "OUTPUT HAS NaN!" : "Output clean");

        juce::Logger::outputDebugString ("=== ALL OK ===");
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

    void shutdown() override {}
};

START_JUCE_APPLICATION (MinimalTest)
