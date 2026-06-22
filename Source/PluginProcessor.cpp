#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CrashLogger.h"

//==============================================================================
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

SpatialReactorAudioProcessor::~SpatialReactorAudioProcessor()
{
}

//==============================================================================
const juce::String SpatialReactorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpatialReactorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpatialReactorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpatialReactorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpatialReactorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpatialReactorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpatialReactorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpatialReactorAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String SpatialReactorAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void SpatialReactorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void SpatialReactorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    sourceClassifier.prepare(sampleRate, samplesPerBlock);
    hybridDecorrelator.prepare(sampleRate, samplesPerBlock);
    textureEngine.prepare(sampleRate, samplesPerBlock);
    airEngine.prepare(sampleRate);
    correlationGovernor.prepare(sampleRate, samplesPerBlock);
    centerLock.prepare(sampleRate);
}

void SpatialReactorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpatialReactorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SpatialReactorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Must have stereo — check BEFORE any channel 1 access
    if (buffer.getNumChannels() < 2)
        return;

    // ---- Parameters ----
    float amount      = apvts.getRawParameterValue("amount")->load();
    float density     = apvts.getRawParameterValue("density")->load();
    float textureMix  = apvts.getRawParameterValue("texture")->load();
    int   textureMode = (int)apvts.getRawParameterValue("textureMode")->load();
    float airDirect   = apvts.getRawParameterValue("air")->load();
    trustMode         = apvts.getRawParameterValue("trustMode")->load() > 0.5f;
    int profileMode   = (int)apvts.getRawParameterValue("profile")->load(); // 0=Auto

    // 1. M/S conversion
    msMatrix.process(buffer);

    // 2. Source classification (only if Auto)
    if (profileMode == 0)
        currentProfile = sourceClassifier.classify(buffer);
    else
        currentProfile = profileMode - 1;

    // 3. Update macro curves
    macroMapper.update(amount, currentProfile, trustMode);

    // 4. Width Engine (decorrelation + spatial density)
    int activeBands = juce::jlimit(1, 4, static_cast<int>(density * 4.0f) + 1);
    hybridDecorrelator.process(buffer, macroMapper.getWidthTarget(), activeBands);

    // 5. Presence Engine — texture mode from discrete selector, depth from knob + macro
    float textureDepth = juce::jlimit(0.0f, 1.0f, textureMix + macroMapper.getTextureTarget());
    textureEngine.setMode(textureMode);
    textureEngine.process(buffer, textureDepth);

    float airAmount = juce::jlimit(0.0f, 1.0f, airDirect + macroMapper.getAirTarget());
    airEngine.process(buffer, airAmount);

    // 6. Center Lock (optional)
    if (centerLockEnabled)
        centerLock.process(buffer);

    // 7. Correlation governor
    correlationGovernor.process(buffer, trustMode);

    // 7b. Compute correlation for GUI meter
    {
        auto* mid  = buffer.getReadPointer (0);
        auto* side = buffer.getReadPointer (1);
        int n = buffer.getNumSamples();
        float sumMM = 0.f, sumSS = 0.f;
        for (int i = 0; i < n; ++i)
        {
            sumMM += mid[i] * mid[i];
            sumSS += side[i] * side[i];
        }
        float rms_m = std::sqrt (sumMM / (float) n);
        float rms_s = std::sqrt (sumSS / (float) n);
        float corr = (rms_m > 1e-8f) ? 1.0f - (rms_s / rms_m) : 1.0f;
        float prev = correlationValue.load (std::memory_order_relaxed);
        correlationValue.store (prev + 0.08f * (corr - prev), std::memory_order_relaxed);
    }

    // 8. Back to L/R
    msMatrix.inverse(buffer);
}

//==============================================================================
bool SpatialReactorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpatialReactorAudioProcessor::createEditor()
{
    CRASH_LOG ("createEditor called");
    auto* editor = new SpatialReactorAudioProcessorEditor (*this);
    CRASH_LOG ("createEditor returning");
    return editor;
}

//==============================================================================
void SpatialReactorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save parameters
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SpatialReactorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore parameters
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr)
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpatialReactorAudioProcessor();
}
