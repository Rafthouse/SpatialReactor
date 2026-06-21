#include <cstdio>
#include <cmath>
#include "../Source/PluginProcessor.h"

// Ultra-minimal test — no JUCEApplication, no message loop
// Just construct the processor and call processBlock

int main()
{
    printf("=== ULTRA MINIMAL TEST ===\n");
    fflush(stdout);

    // Step 1: constructor
    printf("Creating processor...\n");
    fflush(stdout);
    auto* proc = new SpatialReactorAudioProcessor();
    printf("Processor constructor OK\n");
    fflush(stdout);

    // Step 2: prepareToPlay
    printf("prepareToPlay...\n");
    fflush(stdout);
    proc->prepareToPlay(44100.0, 512);
    printf("prepareToPlay OK\n");
    fflush(stdout);

    // Step 3: processBlock
    printf("processBlock...\n");
    fflush(stdout);
    juce::AudioBuffer<float> buf(2, 128);
    for (int s = 0; s < 128; ++s) {
        buf.setSample(0, s, sinf((float)s / 128.0f * 6.283f));
        buf.setSample(1, s, cosf((float)s / 128.0f * 6.283f));
    }
    juce::MidiBuffer midi;
    proc->processBlock(buf, midi);
    printf("processBlock OK\n");
    fflush(stdout);

    // Step 4: check output
    bool ok = true;
    for (int ch = 0; ch < 2; ++ch)
        for (int s = 0; s < 128; ++s)
            if (std::isnan(buf.getSample(ch, s)) || std::isinf(buf.getSample(ch, s)))
                ok = false;
    printf("Output %s\n", ok ? "CLEAN" : "HAS NaN!");
    fflush(stdout);

    // Step 5: cleanup
    delete proc;
    printf("=== ALL OK ===\n");
    fflush(stdout);
    return 0;
}
