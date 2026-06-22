#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "widgets/BauhausRotaryKnob.h"
#include "widgets/BauhausButtonGroup.h"
#include "widgets/BauhausSafeFreeToggle.h"
#include "widgets/BauhausCorrelationBar.h"

//==============================================================================
class BauhausLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BauhausLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float pos, float start, float end,
                           juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour&, bool hover, bool down) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool hover, bool down) override;

    static constexpr auto bg          = 0xff2C2E31;
    static constexpr auto alu         = 0xffA8ADB3;
    static constexpr auto text        = 0xffE8E6E3;
    static constexpr auto accent      = 0xff2D6BFF;
    static constexpr auto textDim     = 0x59E8E6E3;
    static constexpr auto textDimmer  = 0x3FE8E6E3;
};

//==============================================================================
class ReactorCoreComponent : public juce::Component
{
public:
    ReactorCoreComponent() = default;
    void paint (juce::Graphics&) override;
};

//==============================================================================
class SpatialReactorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SpatialReactorAudioProcessorEditor (SpatialReactorAudioProcessor&);
    ~SpatialReactorAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SpatialReactorAudioProcessor& audioProcessor;
    BauhausLookAndFeel bauhausLF;

    // ===== НОВІ КАСТОМНІ ВІДЖЕТИ =====

    // Rotary knobs
    std::unique_ptr<BauhausRotaryKnob> amountKnob;
    std::unique_ptr<BauhausRotaryKnob> densityKnob;
    std::unique_ptr<BauhausRotaryKnob> textureKnob;
    std::unique_ptr<BauhausRotaryKnob> airKnob;

    // Button groups
    std::unique_ptr<BauhausButtonGroup> textureGroup;   // Silk/Dust/Rust
    std::unique_ptr<BauhausButtonGroup> profileGroup;    // Auto/Vocal/Inst/Master/Ambient

    // Toggle + meter
    std::unique_ptr<BauhausSafeFreeToggle>  modeToggle; // SAFE/FREE
    std::unique_ptr<BauhausCorrelationBar>  corrBar;    // correlation meter

    // APVTS Attachments
    std::unique_ptr<BauhausRotaryKnob::Attachment> amountAttachment;
    std::unique_ptr<BauhausRotaryKnob::Attachment> densityAttachment;
    std::unique_ptr<BauhausRotaryKnob::Attachment> textureAttachment;
    std::unique_ptr<BauhausRotaryKnob::Attachment> airAttachment;

    std::unique_ptr<BauhausButtonGroup::Attachment>  textureGroupAttachment;
    std::unique_ptr<BauhausButtonGroup::Attachment>  profileGroupAttachment;

    std::unique_ptr<BauhausSafeFreeToggle::Attachment> modeToggleAttachment;

    // Background labels (non-interactive)
    juce::Label titleLabel, subtitleLabel;
    juce::Label signatureLabel;
    juce::Label modeLabel, corrLabel;

    // Reactor core
    ReactorCoreComponent reactorCore;

    // Cached noise texture (created lazily on first paint)
    juce::Image noiseImage;

    // Lazy font initialisation flag — fonts need GDI which requires a HWND.
    // HWND is only available after resized() is called.
    bool fontsInitialized = false;

    // Flag to prevent resized() from running before constructor completes
    bool constructorDone = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpatialReactorAudioProcessorEditor)
};
