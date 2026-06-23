#include "BauhausCorrelationBar.h"

BauhausCorrelationBar::BauhausCorrelationBar() {}
BauhausCorrelationBar::~BauhausCorrelationBar() = default;

void BauhausCorrelationBar::setValue (float newValue)
{
    value = juce::jlimit (-1.0f, 1.0f, newValue);
    repaint();
}

void BauhausCorrelationBar::setLabel (const juce::String& label) { labelText = label; }
void BauhausCorrelationBar::setNumSegments (int n)               { numSegments = juce::jlimit (8, 40, n); repaint(); }

void BauhausCorrelationBar::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    float labelH = 12.0f;

    // Label
    if (labelText.isNotEmpty())
    {
        auto labelArea = area.removeFromTop (labelH);
        g.setColour (juce::Colour (0x3FE8E6E3));
        g.setFont (juce::Font (7.5f, juce::Font::bold));
        g.drawText (labelText, labelArea, juce::Justification::centredLeft);
    }

    // Track
    auto trackArea = area.reduced (0, 3);
    float segWidth = (trackArea.getWidth() - (float) (numSegments - 1) * 1.5f) / (float) numSegments;
    float segGap = 1.5f;
    float displayVal = value * 0.5f + 0.5f;  // -1→0, 0→0.5, +1→1

    for (int i = 0; i < numSegments; ++i)
    {
        float t = (float) i / (float) (numSegments - 1);
        bool lit = (t <= displayVal);

        auto seg = juce::Rectangle<float> (
            trackArea.getX() + i * (segWidth + segGap),
            trackArea.getY(), segWidth, trackArea.getHeight());

        if (lit)
        {
            // Color zones: safe (blue) → caution → critical
            float zone = (float) i / (float) numSegments;
            if (zone < 0.65f)
                g.setColour (juce::Colour (0xff2D6BFF));           // Blue (safe)
            else if (zone < 0.85f)
                g.setColour (juce::Colour (0xffD89A2B));           // Amber (caution)
            else
                g.setColour (juce::Colour (0xffE84A3B));           // Red (critical)
        }
        else
        {
            g.setColour (juce::Colour (0xffA8ADB3).withAlpha (0.07f));
        }

        g.fillRoundedRectangle (seg, 0.8f);
    }

    // dB labels
    {
        int dbVals[] = { -24, -12, -6, 0 };
        int numDb = 4;
        for (int i = 0; i < numDb; ++i)
        {
            float t = (float) i / (float) (numDb - 1);
            float lx = trackArea.getX() + t * trackArea.getWidth();
            g.setFont (juce::Font (6.5f));
            g.setColour (juce::Colour (0x3FE8E6E3));
            g.drawText (juce::String (dbVals[i]),
                        juce::Rectangle<float> (lx - 10, trackArea.getBottom() + 2, 20, 10),
                        juce::Justification::centred);
        }
    }

    // Value number
    auto valArea = area.withLeft (trackArea.getRight() + 8).withWidth (50.0f);
    g.setColour (juce::Colour (0xff2D6BFF));
    g.setFont (juce::Font (10.0f, juce::Font::bold));
    g.drawText (juce::String::formatted ("%.2f", (double) value),
                valArea, juce::Justification::centredLeft);
}

void BauhausCorrelationBar::resized()
{
}
