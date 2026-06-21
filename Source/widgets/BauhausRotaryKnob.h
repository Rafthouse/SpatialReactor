#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class BauhausRotaryKnob : public juce::Component
{
public:
    BauhausRotaryKnob();
    ~BauhausRotaryKnob() override;

    void setRange (double min, double max, double interval);
    void setValue (double newValue, juce::NotificationType notification = juce::dontSendNotification);
    double getValue() const noexcept { return value; }
    void setLabel (const juce::String& label);
    void setValueText (const juce::String& text);
    int getKnobSize() const noexcept { return knobSize; }
    void setKnobSize (int size) { knobSize = size; }

    enum ScaleStyle { dBScale, PercentScale, TextureScale };
    void setScaleStyle (ScaleStyle style) { scaleStyle = style; }

    std::function<void(double)> onValueChange;

    class Attachment
    {
    public:
        Attachment (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, BauhausRotaryKnob& knob);
        ~Attachment();
    private:
        class Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    double value = 0.0;
    double min = 0.0, max = 1.0, interval = 0.01;
    double startDragValue = 0.0;
    int knobSize = 64;
    juce::String labelText;
    juce::String valueText;
    ScaleStyle scaleStyle = PercentScale;

    std::unique_ptr<juce::Drawable> knobBody;

    void drawScale (juce::Graphics& g, juce::Rectangle<float> bounds);
    float valueToAngle() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BauhausRotaryKnob)
};
