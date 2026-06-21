#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
    A 20-segment LED correlation meter with dB-scale markings.
    Mirrors the RatoLEDBar concept but adapted for stereo correlation display.
*/
class BauhausCorrelationBar : public juce::Component
{
public:
    //==============================================================================
    BauhausCorrelationBar();
    ~BauhausCorrelationBar() override;

    /** Set correlation value (-1..1 range, displayed as 0..1 or -1..+1). */
    void setValue (float newValue);
    float getValue() const noexcept { return value; }

    /** Set the label text (e.g. "CORRELATION"). */
    void setLabel (const juce::String& label);

    /** Set the number of segments (default 20). */
    void setNumSegments (int n);

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    float value = 0.68f;
    int numSegments = 20;
    juce::String labelText = "CORRELATION";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BauhausCorrelationBar)
};
