#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace SR::Layout
{
    static constexpr int kW = 900;
    static constexpr int kH = 560;

    static inline juce::Rectangle<int> innerFrame()       { return { 13, 13, 874, 534 }; }
    static inline juce::Rectangle<int> header()           { return { 0, 0, 900, 76 }; }
    static inline juce::Rectangle<int> mainContent()      { return { 28, 94, 844, 374 }; }
    static inline juce::Rectangle<int> reactorModule()    { return { 28, 94, 483, 374 }; }
    static inline juce::Rectangle<int> controlsModule()   { return { 529, 94, 343, 374 }; }
    static inline juce::Rectangle<int> footer()           { return { 0, 484, 900, 76 }; }

    // Reactor core
    static constexpr int coreX = 69, coreY = 161, coreW = 260, coreH = 260;
    static inline juce::Rectangle<int> coreBounds() { return { coreX, coreY, coreW, coreH }; }
    static constexpr int amountKnobSize = 92;

    // Readout strip (right side of reactor module)
    static inline juce::Rectangle<int> readoutStrip() { return { 359, 132, 132, 318 }; }

    // Controls module knobs (centers relative to editor)
    static constexpr int smallKnobSize = 66;
    static constexpr int densityKnobCx = 594,  densityKnobCy = 165;
    static constexpr int textureKnobCx = 701,  textureKnobCy = 165;
    static constexpr int airKnobCx     = 807,  airKnobCy     = 165;

    // Texture mode selector
    static inline juce::Rectangle<int> textureModes() { return { 545, 300, 311, 58 }; }

    // Trust governor
    static inline juce::Rectangle<int> trustZone()    { return { 545, 370, 311, 84 }; }
    static constexpr int trustToggleW = 128, trustToggleH = 50;

    // Footer sub-areas
    static inline juce::Rectangle<int> corrCopyArea()  { return { 28, 492, 118, 52 }; }
    static inline juce::Rectangle<int> corrMeterArea() { return { 160, 498, 520, 28 }; }
    static inline juce::Rectangle<int> signatureArea() { return { 700, 492, 172, 52 }; }

    // Profile selector in header
    static inline juce::Rectangle<int> profileSelector() { return { 536, 34, 336, 28 }; }

    // Screws
    struct ScrewPos { int x, y; };
    static constexpr ScrewPos screws[] = {
        { 18, 18 }, { 874, 18 }, { 18, 534 }, { 874, 534 }
    };
}
