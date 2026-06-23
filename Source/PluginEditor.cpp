#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CrashLogger.h"

SpatialReactorAudioProcessorEditor::SpatialReactorAudioProcessorEditor (SpatialReactorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    CRASH_LOG ("Editor ctor: START");

    amountKnob .setLabel ("AMOUNT");
    amountKnob .setDefaultValue (0.0f);
    densityKnob.setLabel ("DENSITY");
    densityKnob.setDefaultValue (0.5f);
    textureKnob.setLabel ("TEXTURE");
    textureKnob.setDefaultValue (0.0f);
    airKnob    .setLabel ("AIR");
    airKnob    .setDefaultValue (0.0f);

    addAndMakeVisible (faceplate);
    addAndMakeVisible (reactorCore);
    addAndMakeVisible (amountKnob);
    addAndMakeVisible (densityKnob);
    addAndMakeVisible (textureKnob);
    addAndMakeVisible (airKnob);
    addAndMakeVisible (profileSelector);
    addAndMakeVisible (textureSelector);
    addAndMakeVisible (trustToggle);
    addAndMakeVisible (corrMeter);
    addAndMakeVisible (readout);

    amountAtt      = std::make_unique<SpatialRotaryKnob::Attachment> (audioProcessor.apvts, "amount",      amountKnob);
    densityAtt     = std::make_unique<SpatialRotaryKnob::Attachment> (audioProcessor.apvts, "density",     densityKnob);
    textureAtt     = std::make_unique<SpatialRotaryKnob::Attachment> (audioProcessor.apvts, "texture",     textureKnob);
    airAtt         = std::make_unique<SpatialRotaryKnob::Attachment> (audioProcessor.apvts, "air",         airKnob);
    profileAtt     = std::make_unique<SegmentedSelector::Attachment> (audioProcessor.apvts, "profile",     profileSelector);
    textureModeAtt = std::make_unique<SegmentedSelector::Attachment> (audioProcessor.apvts, "textureMode", textureSelector);
    trustAtt       = std::make_unique<TrustToggle::Attachment>       (audioProcessor.apvts, "trustMode",   trustToggle);

    CRASH_LOG ("Editor ctor: attachments OK");

    setSize (SR::Layout::kW, SR::Layout::kH);
    startTimerHz (24);
    CRASH_LOG ("Editor ctor: END");
}

SpatialReactorAudioProcessorEditor::~SpatialReactorAudioProcessorEditor()
{
    stopTimer();
}

void SpatialReactorAudioProcessorEditor::timerCallback()
{
    float corr = audioProcessor.correlationValue.load (std::memory_order_relaxed);
    corrMeter.setValue (corr);

    float amt = audioProcessor.apvts.getRawParameterValue ("amount")->load();
    reactorCore.setAmount (amt);
    readout.setWidth (amt);
    readout.setFieldBars (amt);

    static const juce::StringArray profileNames { "AUTO", "VOCAL", "INST", "MASTER", "AMBIENT" };
    static const juce::StringArray modeNames { "SILK", "DUST", "RUST" };

    int profileIdx = (int) audioProcessor.apvts.getRawParameterValue ("profile")->load();
    int modeIdx    = (int) audioProcessor.apvts.getRawParameterValue ("textureMode")->load();

    readout.setProfile (profileNames[juce::jlimit (0, profileNames.size() - 1, profileIdx)]);
    readout.setMode (modeNames[juce::jlimit (0, modeNames.size() - 1, modeIdx)]);
}

void SpatialReactorAudioProcessorEditor::paint (juce::Graphics&)
{
}

void SpatialReactorAudioProcessorEditor::resized()
{
    using namespace SR::Layout;

    faceplate.setBounds (0, 0, kW, kH);

    reactorCore.setBounds (coreBounds());

    int akS = amountKnobSize;
    int akX = coreX + coreW / 2 - akS / 2;
    int akY = coreY + coreH / 2 - akS / 2 - 8;
    amountKnob.setBounds (akX, akY, akS, akS + 32);

    readout.setBounds (readoutStrip());

    int sk = smallKnobSize;
    densityKnob .setBounds (densityKnobCx - sk / 2,  densityKnobCy - sk / 2,  sk, sk + 32);
    textureKnob .setBounds (textureKnobCx - sk / 2,  textureKnobCy - sk / 2,  sk, sk + 32);
    airKnob     .setBounds (airKnobCx - sk / 2,      airKnobCy - sk / 2,      sk, sk + 32);

    profileSelector.setBounds (SR::Layout::profileSelector());

    textureSelector.setBounds (textureModes());

    auto tz = trustZone();
    int ttX = tz.getX() + (tz.getWidth() - trustToggleW) / 2;
    int ttY = tz.getY() + (tz.getHeight() - trustToggleH) / 2;
    trustToggle.setBounds (ttX, ttY, trustToggleW, trustToggleH);

    corrMeter.setBounds (corrMeterArea());
}
