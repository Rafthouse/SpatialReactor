#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace SR
{
    namespace Colour
    {
        static constexpr juce::uint32 background     = 0xff17191c;
        static constexpr juce::uint32 panel          = 0xff2c2e31;
        static constexpr juce::uint32 panelSecondary = 0xff242629;
        static constexpr juce::uint32 aluminium      = 0xffa8adb3;
        static constexpr juce::uint32 aluminiumDim   = 0xff666b71;
        static constexpr juce::uint32 ink            = 0xffe8e6e3;
        static constexpr juce::uint32 muted          = 0xff95999f;
        static constexpr juce::uint32 accent         = 0xff2d6bff;
        static constexpr juce::uint32 deep           = 0xff101114;
        static constexpr juce::uint32 danger         = 0xffd85a49;

        static inline juce::Colour accentSoft()   { return juce::Colour (accent).withAlpha (0.18f); }
        static inline juce::Colour line()         { return juce::Colour (aluminium).withAlpha (0.32f); }
        static inline juce::Colour moduleBg()     { return juce::Colour (0x0a0b0d).withAlpha (0.18f); }
    }

    namespace Font
    {
        static inline juce::Font sans (float size, int style = juce::Font::plain)
        {
            return juce::Font (juce::FontOptions ("Segoe UI", size, style));
        }
        static inline juce::Font mono (float size, int style = juce::Font::plain)
        {
            return juce::Font (juce::FontOptions ("Consolas", size, style));
        }
    }
}
