#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "GUI/GuiTheme.h"
#include "GUI/GuiLayout.h"
#include "GUI/FaceplateComponent.h"
#include "GUI/ReactorCoreComponent.h"
#include "GUI/SpatialRotaryKnob.h"
#include "GUI/SegmentedSelector.h"
#include "GUI/TrustToggle.h"
#include "GUI/CorrelationMeter.h"
#include "GUI/ReadoutStrip.h"

class SpatialReactorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    explicit SpatialReactorAudioProcessorEditor (SpatialReactorAudioProcessor&);
    ~SpatialReactorAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    SpatialReactorAudioProcessor& audioProcessor;

    // Static faceplate (painted first, non-interactive)
    FaceplateComponent faceplate;

    // Reactor core visualization
    ReactorCoreComponent reactorCore;

    // Knobs
    SpatialRotaryKnob amountKnob   { SR::Layout::amountKnobSize };
    SpatialRotaryKnob densityKnob  { SR::Layout::smallKnobSize };
    SpatialRotaryKnob textureKnob  { SR::Layout::smallKnobSize };
    SpatialRotaryKnob airKnob      { SR::Layout::smallKnobSize };

    // Selectors
    SegmentedSelector profileSelector { { "AUTO", "VOCAL", "INST", "MASTER", "AMBIENT" } };
    SegmentedSelector textureSelector { { "SILK", "DUST", "RUST" } };

    // Trust toggle
    TrustToggle trustToggle;

    // Correlation meter
    CorrelationMeter corrMeter;

    // Readout strip
    ReadoutStrip readout;

    // APVTS Attachments
    std::unique_ptr<SpatialRotaryKnob::Attachment> amountAtt;
    std::unique_ptr<SpatialRotaryKnob::Attachment> densityAtt;
    std::unique_ptr<SpatialRotaryKnob::Attachment> textureAtt;
    std::unique_ptr<SpatialRotaryKnob::Attachment> airAtt;
    std::unique_ptr<SegmentedSelector::Attachment> profileAtt;
    std::unique_ptr<SegmentedSelector::Attachment> textureModeAtt;
    std::unique_ptr<TrustToggle::Attachment>       trustAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpatialReactorAudioProcessorEditor)
};
