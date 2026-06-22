#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "GuiTheme.h"

class SpatialRotaryKnob : public juce::Component
{
public:
    explicit SpatialRotaryKnob (int knobDiameter = 66);
    ~SpatialRotaryKnob() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

    void   setValue (float newValue, juce::NotificationType n = juce::sendNotification);
    float  getValue() const { return currentValue; }

    void setLabel (const juce::String& text);
    void setDefaultValue (float v) { defaultValue = v; }

    std::function<void (float)> onValueChange;

    // APVTS attachment
    class Attachment
    {
    public:
        Attachment (juce::AudioProcessorValueTreeState&, const juce::String& paramID, SpatialRotaryKnob&);
        ~Attachment();
    private:
        class Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

private:
    int    diameter;
    float  currentValue  = 0.5f;
    float  defaultValue  = 0.5f;
    float  dragStartVal  = 0.0f;
    juce::String labelText;

    static constexpr float kStartAngle = juce::MathConstants<float>::pi * (-132.0f / 180.0f);
    static constexpr float kEndAngle   = juce::MathConstants<float>::pi * ( 132.0f / 180.0f);

    float valueToAngle (float v) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpatialRotaryKnob)
};
