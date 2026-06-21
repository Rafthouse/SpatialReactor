#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

/**
    MacroMapper — the heart of Spatial Reactor.

    Translates the global Amount parameter + active Profile into per-engine
    targets (Width, Texture, Air) using pre-defined curves per profile.

    In Trust Mode all targets are scaled down for safer/more transparent results.
*/
class MacroMapper
{
public:
    /** Recalculate all targets.
        @param amount    Global Amount [0..1]
        @param profile   Profile index: 0=Vocal, 1=Instrument, 2=Master, 3=Ambient
        @param trustMode If true, scale down targets for safety
    */
    void update(float amount, int profile, bool trustMode)
    {
        jassert(profile >= 0 && profile <= 3);

        // Width curves per profile
        widthTarget  = widthCurves[profile].evaluate(amount);
        textureTarget = textureCurves[profile].evaluate(amount);
        airTarget     = airCurves[profile].evaluate(amount);

        if (trustMode)
        {
            widthTarget   *= 0.8f;
            textureTarget *= 0.7f;
            airTarget     *= 0.8f;
        }
    }

    float getWidthTarget()   const noexcept { return widthTarget; }
    float getTextureTarget() const noexcept { return textureTarget; }
    float getAirTarget()     const noexcept { return airTarget; }

private:
    // A simple curve evaluator using a polynomial mapping
    struct Curve
    {
        float base     = 0.0f;  // value at amount=0
        float exponent = 1.0f;  // shape of the curve
        float scale    = 1.0f;  // max value

        float evaluate(float amount) const
        {
            return base + scale * std::pow(amount, exponent);
        }
    };

    float widthTarget   = 0.0f;
    float textureTarget = 0.0f;
    float airTarget     = 0.0f;

    // --- Per-profile curves ---
    // Index order: 0=Vocal, 1=Instrument, 2=Master, 3=Ambient

    std::array<Curve, 4> widthCurves = { {
        /* Vocal */      { 0.0f, 1.2f, 0.70f },
        /* Instrument */ { 0.0f, 1.0f, 0.50f },
        /* Master */     { 0.0f, 2.0f, 0.20f },
        /* Ambient */    { 0.0f, 0.8f, 0.85f }
    } };

    std::array<Curve, 4> textureCurves = { {
        /* Vocal */      { 0.0f, 1.5f, 0.20f },
        /* Instrument */ { 0.0f, 1.0f, 0.35f },
        /* Master */     { 0.0f, 2.0f, 0.10f },
        /* Ambient */    { 0.0f, 0.8f, 0.55f }
    } };

    std::array<Curve, 4> airCurves = { {
        /* Vocal */      { 0.0f, 1.2f, 0.10f },
        /* Instrument */ { 0.0f, 1.0f, 0.25f },
        /* Master */     { 0.0f, 1.8f, 0.70f },
        /* Ambient */    { 0.0f, 0.8f, 0.45f }
    } };
};
