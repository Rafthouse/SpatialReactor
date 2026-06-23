#include "CorrelationMeter.h"

CorrelationMeter::CorrelationMeter()  {}
CorrelationMeter::~CorrelationMeter() = default;

void CorrelationMeter::setValue (float v)
{
    currentValue = juce::jlimit (-1.0f, 1.0f, v);
    repaint();
}

void CorrelationMeter::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    // How many segments are active: map -1..+1 -> 0..32
    int activeCount = juce::roundToInt ((currentValue + 1.0f) * 0.5f * (float) kSegments);
    activeCount = juce::jlimit (0, kSegments, activeCount);

    float totalWidth   = area.getWidth();
    float totalGaps    = (float) (kSegments - 1) * kSegGap;
    float segW         = (totalWidth - totalGaps) / (float) kSegments;
    float segH         = area.getHeight();

    for (int i = 0; i < kSegments; ++i)
    {
        float x = area.getX() + (float) i * (segW + kSegGap);
        juce::Rectangle<float> seg (x, area.getY(), segW, segH);

        bool on = (i < activeCount);

        if (on)
        {
            // First 5 segments = warn/danger zone when lit
            if (i < kWarnSegs)
                g.setColour (juce::Colour (SR::Colour::danger));
            else
                g.setColour (juce::Colour (SR::Colour::accent));
        }
        else
        {
            // Off segment: dark fill + dim border
            g.setColour (juce::Colour (SR::Colour::background));
            g.fillRect (seg);
            g.setColour (juce::Colour (SR::Colour::aluminium).withAlpha (0.18f));
            g.drawRect (seg, 1.0f);
            continue; // skip fill below
        }

        g.fillRect (seg);
    }
}

void CorrelationMeter::resized() {}
