#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "dsp/MSMatrix.h"
#include "dsp/SourceClassifier.h"
#include "dsp/HybridDecorrelator.h"
#include "dsp/TextureEngine.h"
#include "dsp/AirEngine.h"
#include "dsp/CorrelationGovernor.h"
#include "dsp/CenterLock.h"
#include "dsp/MacroMapper.h"

//==============================================================================
class SpatialReactorAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    SpatialReactorAudioProcessor();
    ~SpatialReactorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;

    // Expose DSP state for the editor
    int getCurrentProfile() const noexcept { return currentProfile; }

private:
    // DSP blocks
    MSMatrix msMatrix;
    SourceClassifier sourceClassifier;
    HybridDecorrelator hybridDecorrelator;
    TextureEngine textureEngine;
    AirEngine airEngine;
    CorrelationGovernor correlationGovernor;
    CenterLock centerLock;
    MacroMapper macroMapper;

    // State
    int currentProfile = 0; // 0:Vocal, 1:Instrument, 2:Master, 3:Ambient
    bool trustMode = true;
    bool centerLockEnabled = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpatialReactorAudioProcessor)
};
