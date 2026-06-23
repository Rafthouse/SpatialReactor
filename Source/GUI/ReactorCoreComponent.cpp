#include "ReactorCoreComponent.h"

ReactorCoreComponent::ReactorCoreComponent()
{
    setInterceptsMouseClicks (false, false);
}

ReactorCoreComponent::~ReactorCoreComponent() = default;

void ReactorCoreComponent::setAmount (float t)
{
    t = juce::jlimit (0.0f, 1.0f, t);
    if (std::abs (t - amount) > 0.001f)
    {
        amount = t;
        // Trigger glow cache rebuild
        if (std::abs (t - cachedAmount) > 0.01f)
            glowCache = juce::Image(); // invalidate
        repaint();
    }
}

void ReactorCoreComponent::rebuildGlowCache()
{
    int w = getWidth();
    int h = getHeight();
    if (w <= 0 || h <= 0) return;

    glowCache = juce::Image (juce::Image::ARGB, w, h, true);
    juce::Graphics gc (glowCache);

    float cx = (float) w * 0.5f;
    float cy = (float) h * 0.5f;

    // Glow radius: 170px * (0.76 + t*0.38), scaled to component
    float scale = (float) juce::jmin (w, h) / kBaseDiameter;
    float glowRadius = 170.0f * scale * (0.76f + amount * 0.38f);
    float glowAlpha  = 0.52f * amount;

    juce::ColourGradient grad (
        juce::Colour (SR::Colour::accent).withAlpha (glowAlpha * 0.28f),
        cx, cy,
        juce::Colour (SR::Colour::accent).withAlpha (0.0f),
        cx + glowRadius, cy,
        true); // radial

    gc.setGradientFill (grad);
    gc.fillEllipse (cx - glowRadius, cy - glowRadius, glowRadius * 2.0f, glowRadius * 2.0f);

    cachedAmount = amount;
}

void ReactorCoreComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();

    float scale = (float) juce::jmin ((int) bounds.getWidth(), (int) bounds.getHeight()) / kBaseDiameter;

    float t = amount;

    // --- Cross axes: 238px, aluminium 18% ---
    g.setColour (juce::Colour (SR::Colour::aluminium).withAlpha (0.18f));
    float axisHalfLen = 119.0f * scale;
    g.drawLine (cx - axisHalfLen, cy, cx + axisHalfLen, cy, 1.0f);
    g.drawLine (cx, cy - axisHalfLen, cx, cy + axisHalfLen, 1.0f);

    // --- Outer tick marks at ~77% radius ---
    {
        float tickR     = 112.0f * scale;  // ~76-78% of 148 (half of 238 + some margin)
        float tickInner = tickR * 0.93f;
        g.setColour (juce::Colour (SR::Colour::aluminium).withAlpha (0.42f));
        for (int i = 0; i < 36; ++i)
        {
            float angle = (float) i * juce::MathConstants<float>::twoPi / 36.0f;
            float sinA  = std::sin (angle);
            float cosA  = std::cos (angle);
            g.drawLine (cx + sinA * tickInner, cy - cosA * tickInner,
                        cx + sinA * tickR, cy - cosA * tickR, 1.0f);
        }
    }

    // --- Core mark ring: 224px diameter, conic marks at specific angles ---
    {
        float markR = 112.0f * scale;
        // Cobalt marks at 0, 90, 180, 270 deg; ivory at 45, 135, 225, 315
        int cardinalAngles[] = { 0, 90, 180, 270 };
        int diagonalAngles[] = { 45, 135, 225, 315 };

        for (int a : cardinalAngles)
        {
            float rad  = juce::degreesToRadians ((float) a - 90.0f); // offset so 0=top
            float sinA = std::sin (rad);
            float cosA = std::cos (rad);
            float r1   = markR - 8.0f * scale;
            float r2   = markR;
            g.setColour (juce::Colour (SR::Colour::accent));
            g.drawLine (cx + sinA * r1, cy - cosA * r1, cx + sinA * r2, cy - cosA * r2, 2.0f);
        }

        for (int a : diagonalAngles)
        {
            float rad  = juce::degreesToRadians ((float) a - 90.0f);
            float sinA = std::sin (rad);
            float cosA = std::cos (rad);
            float r1   = markR - 5.0f * scale;
            float r2   = markR;
            g.setColour (juce::Colour (SR::Colour::ink).withAlpha (0.72f));
            g.drawLine (cx + sinA * r1, cy - cosA * r1, cx + sinA * r2, cy - cosA * r2, 1.5f);
        }
    }

    // --- Glow (cached image) ---
    if (!glowCache.isValid() || std::abs (amount - cachedAmount) > 0.01f)
        rebuildGlowCache();

    if (glowCache.isValid())
        g.drawImage (glowCache, 0, 0, getWidth(), getHeight(), 0, 0, glowCache.getWidth(), glowCache.getHeight());

    // --- 4 concentric rings ---
    // r1: 212px base, aluminium-dim, opacity 0.48
    {
        float r = 106.0f * scale * (0.76f + t * 0.20f);
        auto col = juce::Colour (SR::Colour::aluminiumDim).withAlpha (0.48f);
        g.setColour (col);
        g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.0f);
    }

    // r2: 170px base, accent 75% alpha, opacity 0.64
    {
        float r = 85.0f * scale * (0.74f + t * 0.30f);
        auto col = juce::Colour (SR::Colour::accent).withAlpha (0.64f * 0.75f);
        g.setColour (col);
        g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.0f);
    }

    // r3: 126px base, aluminium-dim, opacity 0.90
    {
        float r = 63.0f * scale * (0.72f + t * 0.42f);
        auto col = juce::Colour (SR::Colour::aluminiumDim).withAlpha (0.90f);
        g.setColour (col);
        g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.0f);
    }

    // r4: 80px base, accent, opacity 0.95
    {
        float r = 40.0f * scale * (0.72f + t * 0.50f);
        auto col = juce::Colour (SR::Colour::accent).withAlpha (0.95f);
        g.setColour (col);
        g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.5f);
    }

    // --- Width scale bar at bottom ---
    {
        float barY    = bounds.getBottom() - 18.0f;
        float barLeft = cx - axisHalfLen;
        float barRight = cx + axisHalfLen;
        float barW    = barRight - barLeft;

        // Gradient line: aluminium-dim -> accent -> aluminium-dim
        juce::ColourGradient lineGrad (
            juce::Colour (SR::Colour::aluminiumDim), barLeft, barY,
            juce::Colour (SR::Colour::aluminiumDim), barRight, barY,
            false);
        lineGrad.addColour (0.5, juce::Colour (SR::Colour::accent));
        g.setGradientFill (lineGrad);
        g.drawLine (barLeft, barY, barRight, barY, 1.0f);

        // Labels below the bar
        g.setFont (SR::Font::sans (7.0f));
        g.setColour (juce::Colour (SR::Colour::muted));
        g.drawText ("MONO", juce::Rectangle<float> (barLeft - 10.0f, barY + 3.0f, 32.0f, 10.0f),
                    juce::Justification::centredLeft, false);
        g.drawText ("WIDE", juce::Rectangle<float> (cx - 12.0f, barY + 3.0f, 24.0f, 10.0f),
                    juce::Justification::centred, false);
        g.drawText ("HUGE", juce::Rectangle<float> (barRight - 22.0f, barY + 3.0f, 32.0f, 10.0f),
                    juce::Justification::centredRight, false);
    }
}

void ReactorCoreComponent::resized()
{
    // Invalidate glow cache on resize
    glowCache    = juce::Image();
    cachedAmount = -1.0f;
}
