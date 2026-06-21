#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CrashLogger.h"

//==============================================================================
BauhausLookAndFeel::BauhausLookAndFeel()
{
    CRASH_LOG ("BauhausLookAndFeel::ctor START");
    setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (accent));
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (alu));
    setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    CRASH_LOG ("BauhausLookAndFeel::ctor END");
}

void BauhausLookAndFeel::drawRotarySlider (juce::Graphics&, int, int, int, int,
                                            float, float, float, juce::Slider&) {}
void BauhausLookAndFeel::drawButtonBackground (juce::Graphics&, juce::Button&,
                                                const juce::Colour&, bool, bool) {}
void BauhausLookAndFeel::drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                                            bool, bool) {}

//==============================================================================
void ReactorCoreComponent::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    auto cx = r.getCentreX();
    auto cy = r.getCentreY();
    auto maxR = juce::jmin (r.getWidth(), r.getHeight()) * 0.5f;

    // Outline
    g.setColour (juce::Colour (BauhausLookAndFeel::text).withAlpha (0.08f));
    g.drawEllipse (cx - maxR, cy - maxR, maxR * 2, maxR * 2, 1.5f);

    // Grid ring
    auto gridR = maxR * 0.84f;
    g.setColour (juce::Colour (BauhausLookAndFeel::accent).withAlpha (0.12f));
    g.drawEllipse (cx - gridR, cy - gridR, gridR * 2, gridR * 2, 1.0f);

    // 12 rays
    g.setColour (juce::Colour (BauhausLookAndFeel::accent).withAlpha (0.05f));
    for (int i = 0; i < 12; ++i)
    {
        float a = juce::MathConstants<float>::pi * 2.0f * i / 12.0f;
        g.drawLine (cx, cy, cx + std::sin (a) * maxR * 0.85f,
                    cy - std::cos (a) * maxR * 0.85f, 1.0f);
    }

    // Concentric rings
    float rings[]  = { 0.65f, 0.46f, 0.26f };
    float alphas[] = { 0.2f, 0.3f, 0.45f };
    for (int i = 0; i < 3; ++i)
    {
        auto rr = maxR * rings[i];
        g.setColour (juce::Colour (BauhausLookAndFeel::accent).withAlpha (alphas[i]));
        g.drawEllipse (cx - rr, cy - rr, rr * 2, rr * 2, (i == 1) ? 1.0f : 1.5f);
    }

    // Center dot
    auto dotR = maxR * 0.08f;
    g.setColour (juce::Colour (BauhausLookAndFeel::accent));
    g.fillEllipse (cx - dotR, cy - dotR, dotR * 2, dotR * 2);
}

//==============================================================================
SpatialReactorAudioProcessorEditor::SpatialReactorAudioProcessorEditor (SpatialReactorAudioProcessor& p)
 : AudioProcessorEditor (&p), audioProcessor (p)
{
 // [ЛОГ 1] Початок
 juce::Logger::writeToLog("1. Editor ctor: STARTING (HWND не створено)");

 setLookAndFeel (&bauhausLF);
 juce::Logger::writeToLog("1a. setLookAndFeel OK");
 setSize (700, 960);
 juce::Logger::writeToLog("1b. setSize OK");

 // --- Етап 1: Створення ручок ---
 juce::Logger::writeToLog("2. Editor ctor: Creating amountKnob");
 amountKnob = std::make_unique<BauhausRotaryKnob>();

 juce::Logger::writeToLog("3. Editor ctor: Creating densityKnob");
 densityKnob = std::make_unique<BauhausRotaryKnob>();

 juce::Logger::writeToLog("4. Editor ctor: Creating textureKnob");
 textureKnob = std::make_unique<BauhausRotaryKnob>();

 juce::Logger::writeToLog("5. Editor ctor: Creating airKnob");
 airKnob = std::make_unique<BauhausRotaryKnob>();

 // --- Етап 2: Створення груп та тумблерів ---
 juce::Logger::writeToLog("6. Editor ctor: Creating textureGroup");
 textureGroup = std::make_unique<BauhausButtonGroup>(std::vector<juce::String>{"Silk", "Dust", "Rust"});

 juce::Logger::writeToLog("7. Editor ctor: Creating profileGroup");
 profileGroup = std::make_unique<BauhausButtonGroup>(std::vector<juce::String>{"Auto", "Vocal", "Instrument", "Master", "Ambient"});

 juce::Logger::writeToLog("8. Editor ctor: Creating modeToggle");
 modeToggle = std::make_unique<BauhausSafeFreeToggle>();

 juce::Logger::writeToLog("9. Editor ctor: Creating corrBar");
 corrBar = std::make_unique<BauhausCorrelationBar>();

 // --- Етап 3: Додавання на екран (addAndMakeVisible) ---
 // Цей етап дуже часто падає!
 juce::Logger::writeToLog("10. Editor ctor: addAndMakeVisible - start");

 addAndMakeVisible (amountKnob.get());
 juce::Logger::writeToLog(" -> amountKnob added");

 addAndMakeVisible (densityKnob.get());
 juce::Logger::writeToLog(" -> densityKnob added");

 addAndMakeVisible (textureKnob.get());
 juce::Logger::writeToLog(" -> textureKnob added");

 addAndMakeVisible (airKnob.get());
 juce::Logger::writeToLog(" -> airKnob added");

 addAndMakeVisible (textureGroup.get());
 addAndMakeVisible (profileGroup.get());
 addAndMakeVisible (modeToggle.get());
 addAndMakeVisible (corrBar.get());

 juce::Logger::writeToLog("11. Editor ctor: addAndMakeVisible - END");

 // --- Етап 4: Прив'язка до APVTS (Attachments) ---
 juce::Logger::writeToLog("12. Editor ctor: Creating attachments - start");
 amountAttachment = std::make_unique<BauhausRotaryKnob::Attachment>(audioProcessor.apvts, "amount", *amountKnob);
 densityAttachment = std::make_unique<BauhausRotaryKnob::Attachment>(audioProcessor.apvts, "density", *densityKnob);
 textureAttachment = std::make_unique<BauhausRotaryKnob::Attachment>(audioProcessor.apvts, "texture", *textureKnob);
 airAttachment = std::make_unique<BauhausRotaryKnob::Attachment>(audioProcessor.apvts, "air", *airKnob);
 textureGroupAttachment = std::make_unique<BauhausButtonGroup::Attachment>(audioProcessor.apvts, "texture", *textureGroup);
 profileGroupAttachment = std::make_unique<BauhausButtonGroup::Attachment>(audioProcessor.apvts, "profile", *profileGroup);
 modeToggleAttachment = std::make_unique<BauhausSafeFreeToggle::Attachment>(audioProcessor.apvts, "trustMode", *modeToggle);

 juce::Logger::writeToLog("13. Editor ctor: attachments - END");

 // --- ФІНАЛЬНИЙ ЛОГ ---
 juce::Logger::writeToLog("14. Editor ctor: END");
}

SpatialReactorAudioProcessorEditor::~SpatialReactorAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void SpatialReactorAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    g.fillAll (juce::Colour (BauhausLookAndFeel::bg));

    // Noise/grain overlay (lazy-init on first paint)
    {
        if (!fontsInitialized)
        {
            fontsInitialized = true;
            juce::Logger::writeToLog("paint: lazy font init");
            titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
            subtitleLabel.setFont (juce::Font (8.0f));
            modeLabel.setFont (juce::Font (9.0f, juce::Font::bold));
            signatureLabel.setFont (juce::Font (16.0f, juce::Font::plain));
        }
        if (noiseImage.isNull())
        {
            noiseImage = juce::Image (juce::Image::RGB, 256, 256, false);
            juce::Random rng;
            for (int y = 0; y < 256; ++y)
                for (int x = 0; x < 256; ++x)
                    noiseImage.setPixelAt (x, y, juce::Colour (255, 255, 255).withBrightness (rng.nextFloat()));
        }
        g.setOpacity (0.018f);
        g.drawImageWithin (noiseImage, 0, 0, getWidth(), getHeight(),
                           juce::RectanglePlacement::stretchToFit);
        g.setOpacity (1.0f);
    }

    // Aluminium trim
    g.setGradientFill (juce::ColourGradient::horizontal (
        juce::Colour (0xff8a8f96), juce::Colour (0xffc4c8cc), bounds.toFloat()));
    g.fillRect (0, 0, getWidth(), 4);
    g.fillRect (0, getHeight() - 4, getWidth(), 4);

    // Top bar divider
    g.setColour (juce::Colour (BauhausLookAndFeel::alu).withAlpha (0.12f));
    g.fillRect (0, 70, getWidth(), 1);

    // Screws
    auto drawScrew = [&](int sx, int sy)
    {
        g.setColour (juce::Colour (0xff5a5d62));
        g.fillEllipse ((float) sx, (float) sy, 12.0f, 12.0f);
        g.setColour (juce::Colour (BauhausLookAndFeel::alu).withAlpha (0.15f));
        g.drawEllipse ((float) sx, (float) sy, 12.0f, 12.0f, 1.0f);
        g.setColour (juce::Colour (BauhausLookAndFeel::alu).withAlpha (0.25f));
        g.drawLine ((float) sx + 2, (float) sy + 6, (float) sx + 10, (float) sy + 6, 1.0f);
    };
    drawScrew (14, 14);
    drawScrew (getWidth() - 26, 14);
    drawScrew (14, getHeight() - 78);
    drawScrew (getWidth() - 26, getHeight() - 78);

    // Bottom bar divider
    auto bottomBarArea = getLocalBounds().removeFromBottom (68);
    g.setColour (juce::Colour (BauhausLookAndFeel::alu).withAlpha (0.12f));
    g.fillRect (bottomBarArea.removeFromTop (1));

    // Logo
    auto logoR = juce::Rectangle<int> (18, 16, 40, 40);
    g.setColour (juce::Colour (BauhausLookAndFeel::accent));
    g.drawEllipse (logoR.toFloat(), 1.5f);
    g.drawEllipse (logoR.reduced (5).toFloat(), 1.0f);
    g.drawEllipse (logoR.reduced (11).toFloat(), 1.0f);
    g.fillEllipse (logoR.reduced (17).toFloat());
    g.setColour (juce::Colour (BauhausLookAndFeel::bg));
    g.fillRect (logoR.getX() + 4, logoR.getY(), 10, 3);

    // --- Malevich / Yermylov constructivist accents ---
    // Subtle vertical reference line
    g.setColour (juce::Colour (BauhausLookAndFeel::accent).withAlpha (0.025f));
    g.drawLine (20, 74, 20, getHeight() - 72, 1.0f);

    // Horizontal reference line
    g.drawLine (12, getHeight() / 2, getWidth() - 12, getHeight() / 2, 1.0f);

    // Small constructivist square (bottom-right area)
    auto malevR = juce::Rectangle<int> (getWidth() - 46, getHeight() - 66, 18, 18);
    g.setColour (juce::Colour (BauhausLookAndFeel::accent).withAlpha (0.04f));
    g.drawRect (malevR, 2);
    g.fillRect (malevR.reduced (5));

    // Small circle (lower-left of bottom bar)
    g.drawEllipse (168.0f, (float) getHeight() - 62.0f, 10.0f, 10.0f, 1.5f);
}

void SpatialReactorAudioProcessorEditor::resized()
{
    // Fonts are initialized on first paint() to avoid VST3 re-entrant crash
    juce::Logger::writeToLog("resized: called");

    auto bounds = getLocalBounds();
    bounds.reduce (40, 0);

    titleLabel.setBounds (66, 14, 300, 22);
    subtitleLabel.setBounds (66, 36, 300, 14);

    auto content = getLocalBounds()
        .withTrimmedTop (74)
        .withTrimmedBottom (72);

    // Amount knob
    auto amountArea = content.removeFromTop (170);
    int knobCenterX = amountArea.getCentreX();
    amountKnob->setBounds (knobCenterX - 55, amountArea.getY() + 10, 110, 148);

    content.removeFromTop (10);

    // Reactor core
    auto coreArea = content.removeFromTop (160);
    reactorCore.setBounds (coreArea.getCentreX() - 60, coreArea.getY() + 20, 120, 120);

    content.removeFromTop (20);

    // 3 knobs row
    auto knobsArea = content.removeFromTop (110);
    int knobW = 64;
    int knobGap = 80;
    int totalW = knobW * 3 + knobGap * 2;
    int leftX = knobsArea.getCentreX() - totalW / 2;

    densityKnob->setBounds (leftX,                      knobsArea.getY() + 5, knobW, 94);
    textureKnob->setBounds (leftX + knobW + knobGap,    knobsArea.getY() + 5, knobW, 94);
    airKnob->setBounds     (leftX + (knobW + knobGap) * 2, knobsArea.getY() + 5, knobW, 94);

    content.removeFromTop (5);

    // Texture selector
    auto texArea = content.removeFromTop (36);
    textureGroup->setBounds (texArea.getCentreX() - 150, texArea.getY() + 2, 300, 30);

    content.removeFromTop (10);

    // Signature
    auto sigArea = content.removeFromTop (30);
    signatureLabel.setBounds (sigArea.getCentreX() - 100, sigArea.getY(), 200, 24);

    // Profile row
    auto profArea = content.removeFromTop (40);
    profileGroup->setBounds (profArea.getCentreX() - 200, profArea.getY() + 2, 400, 34);

    // Bottom bar
    auto bottomBar = getLocalBounds().removeFromBottom (68).reduced (32, 0);

    modeLabel.setBounds (bottomBar.getX(), bottomBar.getY() + 6, 40, 14);
    modeToggle->setBounds (bottomBar.getX() + 44, bottomBar.getY() + 2, 80, 34);

    auto corrLeft = bottomBar.getX() + 160;
    int corrWidth = bottomBar.getWidth() - corrLeft - 10;
    corrBar->setBounds (corrLeft, bottomBar.getY() + 6, corrWidth, 28);
}
