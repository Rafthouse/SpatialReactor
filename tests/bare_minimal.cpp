// bare_minimal.cpp - No JUCEApplication, no message loop
// Just construct processor and process audio
// Compile: cl.exe /I..\Source /I..\JUCE\modules bare_minimal.cpp ..\Source\PluginProcessor.cpp

#include <cstdio>
#include <cmath>
#include "../Source/PluginProcessor.h"

int main()
{
    printf("=== BARE MINIMAL TEST ===\n");
    fflush(stdout);
    
    // Need to initialize JUCE's Core (message manager, etc.)
    // For a console app without JUCEApplication, we need MessageManager
    juce::MessageManager::getInstance();
    
    printf("Creating processor...\n"); fflush(stdout);
    
    try {
        auto proc = new SpatialReactorAudioProcessor();
        printf("Processor OK\n"); fflush(stdout);
        
        proc->prepareToPlay(44100.0, 512);
        printf("prepareToPlay OK\n"); fflush(stdout);
        
        juce::AudioBuffer<float> buf(2, 128);
        for (int s = 0; s < 128; ++s) {
            buf.setSample(0, s, sinf((float)s / 128.0f * 6.283f));
            buf.setSample(1, s, cosf((float)s / 128.0f * 6.283f));
        }
        juce::MidiBuffer midi;
        proc->processBlock(buf, midi);
        printf("processBlock OK\n"); fflush(stdout);
        
        bool ok = true;
        for (int ch = 0; ch < 2; ++ch)
            for (int s = 0; s < 128; ++s)
                if (std::isnan(buf.getSample(ch,s)) || std::isinf(buf.getSample(ch,s)))
                    ok = false;
        printf("Output %s\n", ok ? "CLEAN" : "HAS NaN"); fflush(stdout);
        
        delete proc;
        printf("=== ALL OK ===\n"); fflush(stdout);
    }
    catch (...) {
        printf("EXCEPTION\n"); fflush(stdout);
        return 1;
    }
    
    return 0;
}
