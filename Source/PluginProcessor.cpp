#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CrashLogger.h"

SpatialReactorAudioProcessor::SpatialReactorAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS",
      {
          std::make_unique<juce::AudioParameterFloat> ("amount",      "Amount",      0.0f, 1.0f, 0.0f),
          std::make_unique<juce::AudioParameterFloat> ("density",     "Density",     0.0f, 1.0f, 0.5f),
          std::make_unique<juce::AudioParameterFloat> ("texture",     "Texture",     0.0f, 1.0f, 0.0f),
          std::make_unique<juce::AudioParameterChoice>("textureMode", "Texture Mode",
              juce::StringArray{"Silk", "Dust", "Rust"}, 0),
          std::make_unique<juce::AudioParameterFloat> ("air",         "Air",         0.0f, 1.0f, 0.0f),
          std::make_unique<juce::AudioParameterBool>  ("trustMode",   "Trust Mode",  true),
          std::make_unique<juce::AudioParameterChoice>("profile",     "Profile",
              juce::StringArray{"Auto", "Vocal", "Instrument", "Master", "Ambient"}, 0)
      })
{
    CRASH_LOG ("PluginProcessor ctor: END");
}

SpatialReactorAudioProcessor::~SpatialReactorAudioProcessor() {}

const juce::String SpatialReactorAudioProcessor::getName() const { return JucePlugin_Name; }
bool SpatialReactorAudioProcessor::acceptsMidi() const  { return false; }
bool SpatialReactorAudioProcessor::producesMidi() const { return false; }
bool SpatialReactorAudioProcessor::isMidiEffect() const { return false; }
double SpatialReactorAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int SpatialReactorAudioProcessor::getNumPrograms()    { return 1; }
int SpatialReactorAudioProcessor::getCurrentProgram() { return 0; }
void SpatialReactorAudioProcessor::setCurrentProgram (int) {}
const juce::String SpatialReactorAudioProcessor::getProgramName (int) { return {}; }
void SpatialReactorAudioProcessor::changeProgramName (int, const juce::String&) {}

void SpatialReactorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sourceClassifier.prepare (sampleRate, samplesPerBlock);
    spatialGen.prepare (sampleRate, samplesPerBlock);
    textureEngine.prepare (sampleRate, samplesPerBlock);
    airEngine.prepare (sampleRate, samplesPerBlock);
    energyBudget.prepare (sampleRate, samplesPerBlock);
    centerLock.prepare (sampleRate, samplesPerBlock);
    corrGovernor.prepare (sampleRate, samplesPerBlock);
    loudnessComp.prepare (sampleRate, samplesPerBlock);
    balanceGuard.prepare (sampleRate, samplesPerBlock);
    peakLimiter.prepare (sampleRate, samplesPerBlock);
    preAnalyzer.prepare (sampleRate, samplesPerBlock);
    postAnalyzer.prepare (sampleRate, samplesPerBlock);
    dcBlockL.prepare (sampleRate);
    dcBlockR.prepare (sampleRate);
}

void SpatialReactorAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpatialReactorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}
#endif

void SpatialReactorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (buffer.getNumChannels() < 2)
        return;

    // --- Read parameters ---
    float amount      = apvts.getRawParameterValue ("amount")->load();
    float density     = apvts.getRawParameterValue ("density")->load();
    float textureMix  = apvts.getRawParameterValue ("texture")->load();
    int   textureMode = (int) apvts.getRawParameterValue ("textureMode")->load();
    float airDirect   = apvts.getRawParameterValue ("air")->load();
    trustMode         = apvts.getRawParameterValue ("trustMode")->load() > 0.5f;
    int profileMode   = (int) apvts.getRawParameterValue ("profile")->load();

    // --- 0. Capture raw input loudness (before DC blocker) ---
    loudnessComp.captureInput (buffer);

    // --- 1. Input DC guard ---
    {
        auto* l = buffer.getWritePointer (0);
        auto* r = buffer.getWritePointer (1);
        int n = buffer.getNumSamples();
        dcBlockL.processBlock (l, n);
        dcBlockR.processBlock (r, n);
    }

    // --- 2. Pre-DSP analysis + balance capture ---
    preAnalyzer.analyze (buffer);
    balanceGuard.captureInputBalance (buffer);

    // --- 2. Energy-preserving L/R → M/S ---
    EnergyPreservingMS::encode (buffer);

    // --- 3. Source classification ---
    if (profileMode == 0)
        currentProfile = sourceClassifier.classify (buffer);
    else
        currentProfile = profileMode - 1;

    // --- 4. Macro mapper ---
    macroMapper.update (amount, currentProfile, trustMode);

    // --- 5. Capture pre-processing M/S energy ---
    energyBudget.captureInput (buffer);

    // --- 6. Spatial generation (complementary widening) ---
    int activeBands = juce::jlimit (1, 4, (int)(density * 4.f) + 1);
    spatialGen.process (buffer, macroMapper.getWidthTarget(), activeBands);

    // --- 7. Texture engine (oversampled saturation on Side) ---
    float textureDepth = juce::jlimit (0.f, 1.f, textureMix + macroMapper.getTextureTarget());
    textureEngine.setMode (textureMode);
    textureEngine.process (buffer, textureDepth);

    // --- 8. Air engine (HF exciter on Side) ---
    float airAmount = juce::jlimit (0.f, 1.f, airDirect + macroMapper.getAirTarget());
    airEngine.process (buffer, airAmount);

    // --- 9. Energy budget normalization ---
    energyBudget.apply (buffer);

    // --- 10. Center lock (LF mono guard on Side) ---
    centerLock.process (buffer);

    // --- 11. Correlation governor ---
    corrGovernor.process (buffer, trustMode);

    // --- 12. M/S → L/R ---
    EnergyPreservingMS::decode (buffer);

    // --- 13. Channel balance guard ---
    balanceGuard.apply (buffer);

    // --- 14. Loudness compensation ---
    loudnessComp.applyCompensation (buffer);

    // --- 15. True-peak safety ---
    peakLimiter.process (buffer);

    // --- 16. Post-DSP analysis for GUI ---
    auto postSnap = postAnalyzer.analyze (buffer);
    correlationValue.store (postSnap.correlation, std::memory_order_relaxed);
    trimDbValue.store (loudnessComp.getTrimDb(), std::memory_order_relaxed);
}

bool SpatialReactorAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* SpatialReactorAudioProcessor::createEditor()
{
    CRASH_LOG ("createEditor called");
    auto* editor = new SpatialReactorAudioProcessorEditor (*this);
    CRASH_LOG ("createEditor returning");
    return editor;
}

void SpatialReactorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SpatialReactorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr)
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpatialReactorAudioProcessor();
}
