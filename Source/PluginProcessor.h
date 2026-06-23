#pragma once

#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "dsp/EnergyPreservingMS.h"
#include "dsp/DCBlocker.h"
#include "dsp/StereoAnalyzer.h"
#include "dsp/NewSourceClassifier.h"
#include "dsp/ComplementarySpatialGenerator.h"
#include "dsp/NewTextureEngine.h"
#include "dsp/NewAirEngine.h"
#include "dsp/EnergyBudgetMixer.h"
#include "dsp/NewCenterLock.h"
#include "dsp/NewCorrelationGovernor.h"
#include "dsp/LoudnessCompensator.h"
#include "dsp/ChannelBalanceGuard.h"
#include "dsp/TruePeakLimiter.h"
#include "dsp/MacroMapper.h"

class SpatialReactorAudioProcessor : public juce::AudioProcessor
{
public:
    SpatialReactorAudioProcessor();
    ~SpatialReactorAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    int getCurrentProfile() const noexcept { return currentProfile; }

    std::atomic<float> correlationValue { 1.0f };
    std::atomic<float> trimDbValue { 0.0f };

private:
    NewSourceClassifier sourceClassifier;
    ComplementarySpatialGenerator spatialGen;
    NewTextureEngine textureEngine;
    NewAirEngine airEngine;
    EnergyBudgetMixer energyBudget;
    NewCenterLock centerLock;
    NewCorrelationGovernor corrGovernor;
    LoudnessCompensator loudnessComp;
    ChannelBalanceGuard balanceGuard;
    TruePeakLimiter peakLimiter;
    MacroMapper macroMapper;
    StereoAnalyzer preAnalyzer, postAnalyzer;
    DCBlocker dcBlockL, dcBlockR;

    int currentProfile = 0;
    bool trustMode = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpatialReactorAudioProcessor)
};
