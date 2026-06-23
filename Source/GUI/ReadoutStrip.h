#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "GuiTheme.h"

class ReadoutStrip : public juce::Component
{
public:
    ReadoutStrip();
    ~ReadoutStrip() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setWidth   (float w);       // 0..1 normalized, displayed as percentage
    void setProfile (const juce::String& profile);
    void setMode    (const juce::String& mode);
    void setFieldBars (float amount); // 0..1 -> bars 0..7

private:
    float         widthVal   = 0.5f;
    juce::String  profileStr = "WIDE";
    juce::String  modeStr    = "STEREO";
    float         fieldAmount = 0.5f;

    static constexpr int kFieldBars   = 7;
    static constexpr float kRowGap    = 15.0f;
    static constexpr float kKeyH      = 9.0f;   // key label height
    static constexpr float kValH      = 15.0f;  // value label height
    static constexpr float kRowH      = kKeyH + kValH; // total row height

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReadoutStrip)
};
