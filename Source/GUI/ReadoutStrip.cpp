#include "ReadoutStrip.h"

ReadoutStrip::ReadoutStrip()
{
    setInterceptsMouseClicks (false, false);
}

ReadoutStrip::~ReadoutStrip() = default;

void ReadoutStrip::setWidth (float w)
{
    widthVal = juce::jlimit (0.0f, 1.0f, w);
    repaint();
}

void ReadoutStrip::setProfile (const juce::String& profile)
{
    profileStr = profile;
    repaint();
}

void ReadoutStrip::setMode (const juce::String& mode)
{
    modeStr = mode;
    repaint();
}

void ReadoutStrip::setFieldBars (float amount)
{
    fieldAmount = juce::jlimit (0.0f, 1.0f, amount);
    repaint();
}

void ReadoutStrip::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    // --- Left border 1px ---
    g.setColour (SR::Colour::line());
    g.drawLine (area.getX(), area.getY(), area.getX(), area.getBottom(), 1.0f);

    float contentX = area.getX() + 10.0f;
    float contentW = area.getWidth() - 10.0f;
    float y        = area.getY() + 8.0f;

    // Helper: draw one key/value row
    auto drawRow = [&] (const juce::String& key, const juce::String& val, bool cobaltVal)
    {
        // Key label (7px, letter-spacing 0.18em, muted, uppercase)
        g.setFont (SR::Font::sans (7.0f));
        g.setColour (juce::Colour (SR::Colour::muted));
        g.drawText (key.toUpperCase(),
                    juce::Rectangle<float> (contentX, y, contentW, kKeyH),
                    juce::Justification::centredLeft, false);

        y += kKeyH + 2.0f;

        // Value label (13px, mono, bold, ink or cobalt)
        g.setFont (SR::Font::mono (13.0f, juce::Font::bold));
        g.setColour (cobaltVal ? juce::Colour (SR::Colour::accent) : juce::Colour (SR::Colour::ink));
        g.drawText (val,
                    juce::Rectangle<float> (contentX, y, contentW, kValH),
                    juce::Justification::centredLeft, false);

        y += kValH + kRowGap;
    };

    // --- WIDTH ---
    juce::String widthLabel = juce::String (juce::roundToInt (widthVal * 200.0f - 100.0f)) + "%";
    drawRow ("WIDTH", widthLabel, true /* cobalt */);

    // --- PROFILE ---
    drawRow ("PROFILE", profileStr, false);

    // --- MODE ---
    drawRow ("MODE", modeStr, false);

    // --- FIELD ---
    // Key label
    g.setFont (SR::Font::sans (7.0f));
    g.setColour (juce::Colour (SR::Colour::muted));
    g.drawText ("FIELD",
                juce::Rectangle<float> (contentX, y, contentW, kKeyH),
                juce::Justification::centredLeft, false);
    y += kKeyH + 4.0f;

    // 7 mini bars
    {
        float barW  = 5.0f;
        float barGap = 3.0f;
        float maxBarH = 20.0f;
        int   litBars = juce::roundToInt (fieldAmount * (float) kFieldBars);

        for (int i = 0; i < kFieldBars; ++i)
        {
            float barH = maxBarH * (0.3f + (float) i / (float) (kFieldBars - 1) * 0.7f);
            float bx   = contentX + (float) i * (barW + barGap);
            float by   = y + (maxBarH - barH);

            bool lit = (i < litBars);
            g.setColour (lit ? juce::Colour (SR::Colour::accent)
                             : juce::Colour (SR::Colour::aluminiumDim));
            g.fillRect (juce::Rectangle<float> (bx, by, barW, barH));
        }
    }
}

void ReadoutStrip::resized() {}
