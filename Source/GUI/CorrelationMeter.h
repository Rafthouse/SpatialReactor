#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "GuiTheme.h"

class CorrelationMeter : public juce::Component
{
public:
    CorrelationMeter();
    ~CorrelationMeter() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // v in -1..+1. Maps to 0..32 active segments.
    void setValue (float v);
    float getValue() const { return currentValue; }

private:
    static constexpr int kSegments = 32;
    static constexpr int kWarnSegs = 5;   // first 5 use danger color
    static constexpr float kSegGap  = 3.0f;

    float currentValue = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CorrelationMeter)
};
