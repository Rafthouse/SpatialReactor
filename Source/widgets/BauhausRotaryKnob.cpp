#include "BauhausRotaryKnob.h"

//==============================================================================
class BauhausRotaryKnob::Attachment::Pimpl : public juce::AudioProcessorParameter::Listener
{
public:
    Pimpl (juce::AudioProcessorValueTreeState& state, const juce::String& paramID, BauhausRotaryKnob& knob)
        : param (state.getParameter (paramID)), knobRef (knob)
    {
        if (param == nullptr) { jassertfalse; return; }
        knobRef.setValue (param->getValue(), juce::dontSendNotification);
        knobRef.onValueChange = [this] (double v) { param->setValueNotifyingHost ((float) v); };
        param->addListener (this);
    }
    ~Pimpl() {
        knobRef.onValueChange = nullptr;  // prevent dangling callback
        if (param != nullptr)
            param->removeListener (this);
    }
    void parameterValueChanged (int, float newValue) override
    {
        // Capture knob reference directly — outlives Pimpl
        juce::MessageManager::callAsync ([&knob = knobRef, newValue]()
        {
            knob.setValue (newValue, juce::dontSendNotification);
        });
    }
    void parameterGestureChanged (int, bool) override {}
private:
    juce::RangedAudioParameter* param;
    BauhausRotaryKnob& knobRef;
};

BauhausRotaryKnob::Attachment::Attachment (juce::AudioProcessorValueTreeState& state,
                                            const juce::String& paramID, BauhausRotaryKnob& knob)
    : pimpl (std::make_unique<Pimpl> (state, paramID, knob)) {}
BauhausRotaryKnob::Attachment::~Attachment() = default;

//==============================================================================
constexpr float angleStart = juce::MathConstants<float>::pi * 0.75f;
constexpr float angleEnd   = juce::MathConstants<float>::pi * 2.25f;

BauhausRotaryKnob::BauhausRotaryKnob()
{
    setMouseClickGrabsKeyboardFocus (false);
    setWantsKeyboardFocus (true);

    // Knob body is drawn procedurally (no SVG dependency)
    knobBody = nullptr;
}

BauhausRotaryKnob::~BauhausRotaryKnob() = default;

void BauhausRotaryKnob::setRange (double mn, double mx, double intrvl)
{
    min = mn; max = mx; interval = intrvl;
    value = juce::jlimit (0.0, 1.0, (value - min) / (max - min));
}

void BauhausRotaryKnob::setValue (double newValue, juce::NotificationType notification)
{
    value = juce::jlimit (0.0, 1.0, newValue);
    repaint();
    if (notification != juce::dontSendNotification && onValueChange)
        onValueChange (value);
}

void BauhausRotaryKnob::setLabel (const juce::String& label)      { labelText = label; repaint(); }
void BauhausRotaryKnob::setValueText (const juce::String& text)   { valueText = text; repaint(); }

float BauhausRotaryKnob::valueToAngle() const
{
    // Map knob value to rotation: value=0 → indicator at ~7:30, value=0.5 → top, value=1 → ~4:30
    return angleStart + (float) value * (angleEnd - angleStart);
}

void BauhausRotaryKnob::drawScale (juce::Graphics& g, juce::Rectangle<float> area)
{
    auto cx = area.getCentreX();
    auto cy = area.getCentreY();
    auto radius = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f;
    auto totalAngle = angleEnd - angleStart;

    // 72 knurling ticks
    g.setColour (juce::Colour (0xffA8ADB3).withAlpha (0.10f));
    for (int i = 0; i < 72; ++i)
    {
        float t = (float) i / 72.0f;
        float a = angleStart + t * totalAngle;
        float x1 = cx + std::sin (a) * (radius - 3.0f);
        float y1 = cy - std::cos (a) * (radius - 3.0f);
        float x2 = cx + std::sin (a) * (radius + 0.5f);
        float y2 = cy - std::cos (a) * (radius + 0.5f);
        g.drawLine (x1, y1, x2, y2, 0.6f);
    }

    // Major scale ticks (13: -12 to +12)
    int numTicks = 13;
    for (int i = 0; i < numTicks; ++i)
    {
        float t = (float) i / (float) (numTicks - 1);
        float a = angleStart + t * totalAngle;
        bool isCenter = (i == 6);
        float tickLen = isCenter ? 10.0f : 6.0f;
        float outerR = radius + 2.5f;
        float innerR = outerR - tickLen;

        float x1 = cx + std::sin (a) * innerR;
        float y1 = cy - std::cos (a) * innerR;
        float x2 = cx + std::sin (a) * outerR;
        float y2 = cy - std::cos (a) * outerR;

        g.setColour (juce::Colour (0xffA8ADB3).withAlpha (isCenter ? 0.6f : 0.25f));
        g.drawLine (x1, y1, x2, y2, isCenter ? 1.5f : 1.0f);

        if (i == 0 || i == 6 || i == 12)
        {
            float labelR = outerR + 16.0f;
            float lx = cx + std::sin (a) * labelR;
            float ly = cy - std::cos (a) * labelR;
            juce::String lbl = (i == 0) ? "-12" : (i == 6) ? "0" : "+12";
            g.setFont (juce::Font (8.0f));
            g.setColour (juce::Colour (0x59E8E6E3));
            g.drawText (lbl, juce::Rectangle<float> (lx - 12, ly - 6, 24, 12), juce::Justification::centred);
        }
    }
}

void BauhausRotaryKnob::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    auto knobArea = area.withSizeKeepingCentre ((float) knobSize, (float) knobSize);

    // Draw scale around knob
    drawScale (g, knobArea);

    // Draw SVG knob body with rotation if loaded
    if (knobBody != nullptr)
    {
        auto angle = valueToAngle();
        // The SVG indicator dot is at 12 o'clock (angle -3π/2 in valueToAngle)
        // Rotate the SVG so the indicator matches the value
        auto centre = knobArea.getCentre();
        g.addTransform (juce::AffineTransform::rotation (
            angle - juce::MathConstants<float>::halfPi * 3.0f,
            (float) centre.x, (float) centre.y));
        knobBody->drawWithin (g, knobArea.toFloat(), juce::RectanglePlacement::centred, 1.0f);
        g.addTransform (juce::AffineTransform());
    }
    else
    {
        // Fallback: simple circle if SVG not loaded
        auto cx = knobArea.getCentreX();
        auto cy = knobArea.getCentreY();
        auto r = (float) knobSize * 0.5f - 5.0f;
        g.setColour (juce::Colour (0xff2C2E31));
        g.fillEllipse (cx - r, cy - r, r * 2, r * 2);
        g.setColour (juce::Colour (0xffA8ADB3));
        g.drawEllipse (cx - r, cy - r, r * 2, r * 2, 2.5f);
        float angle = valueToAngle();
        float indLen = r * 0.65f;
        g.setColour (juce::Colour (0xff2D6BFF));
        g.drawLine (cx, cy, cx + std::sin (angle) * indLen, cy - std::cos (angle) * indLen, 2.5f);
    }

    // Labels below
    if (labelText.isNotEmpty())
    {
        g.setColour (juce::Colour (0xffE8E6E3));
        g.setFont (juce::Font (10.0f, juce::Font::bold));
        g.drawText (labelText, area.withTop (knobArea.getBottom() + 4), juce::Justification::centred);
    }
    if (valueText.isNotEmpty())
    {
        g.setColour (juce::Colour (0xff2D6BFF));
        g.setFont (juce::Font (10.0f));
        g.drawText (valueText, area.withTop (knobArea.getBottom() + 18), juce::Justification::centred);
    }
}

void BauhausRotaryKnob::resized() {}

void BauhausRotaryKnob::mouseDown (const juce::MouseEvent& e)
{
    startDragValue = value;
    grabKeyboardFocus();
}

void BauhausRotaryKnob::mouseDrag (const juce::MouseEvent& e)
{
    float delta = (float) (e.getDistanceFromDragStartY() * -0.005 + e.getDistanceFromDragStartX() * 0.003);
    setValue (juce::jlimit (0.0, 1.0, startDragValue + delta), juce::sendNotification);
}

void BauhausRotaryKnob::mouseUp (const juce::MouseEvent&) {}

void BauhausRotaryKnob::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    setValue (value + w.deltaY * 0.05, juce::sendNotification);
}
