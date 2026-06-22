#include "SpatialRotaryKnob.h"

//==============================================================================
// Attachment Pimpl
//==============================================================================
class SpatialRotaryKnob::Attachment::Pimpl : public juce::AudioProcessorParameter::Listener
{
public:
    Pimpl (juce::AudioProcessorValueTreeState& state,
           const juce::String& paramID,
           SpatialRotaryKnob& knob)
        : param (state.getParameter (paramID)), knobRef (knob)
    {
        if (param == nullptr) { jassertfalse; return; }
        knobRef.setValue (param->getValue(), juce::dontSendNotification);
        knobRef.onValueChange = [this] (float v) { param->setValueNotifyingHost (v); };
        param->addListener (this);
    }

    ~Pimpl() override
    {
        knobRef.onValueChange = nullptr;
        if (param != nullptr) param->removeListener (this);
    }

    void parameterValueChanged (int, float newValue) override
    {
        juce::MessageManager::callAsync ([&knob = knobRef, newValue]()
        {
            knob.setValue (newValue, juce::dontSendNotification);
        });
    }

    void parameterGestureChanged (int, bool) override {}

private:
    juce::RangedAudioParameter* param;
    SpatialRotaryKnob& knobRef;
};

SpatialRotaryKnob::Attachment::Attachment (juce::AudioProcessorValueTreeState& state,
                                            const juce::String& paramID,
                                            SpatialRotaryKnob& knob)
    : pimpl (std::make_unique<Pimpl> (state, paramID, knob)) {}

SpatialRotaryKnob::Attachment::~Attachment() = default;

//==============================================================================
// SpatialRotaryKnob
//==============================================================================
SpatialRotaryKnob::SpatialRotaryKnob (int knobDiameter)
    : diameter (knobDiameter)
{
    setMouseClickGrabsKeyboardFocus (false);
}

SpatialRotaryKnob::~SpatialRotaryKnob() = default;

float SpatialRotaryKnob::valueToAngle (float v) const
{
    // -132deg at v=0, +132deg at v=1 (measured from 12-o'clock, clockwise positive)
    const float sweep = kEndAngle - kStartAngle; // 264 deg in radians
    return kStartAngle + v * sweep;
}

void SpatialRotaryKnob::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const float cx = bounds.getCentreX();
    const float totalH = bounds.getHeight();
    // knob sits in the top portion, labels in the bottom
    const float labelAreaH = 32.0f;
    const float knobAreaH  = totalH - labelAreaH;
    const float cy = knobAreaH * 0.5f;

    const float r = (float) diameter * 0.5f;

    // --- Dark circle body ---
    g.setColour (juce::Colour (SR::Colour::background));
    g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);

    // --- Inner shadow ring (7px wide, dark) ---
    {
        const float shadowW = 7.0f;
        const float sr2 = r - shadowW * 0.5f;
        g.setColour (juce::Colour (0xff222529));
        g.drawEllipse (cx - sr2, cy - sr2, sr2 * 2.0f, sr2 * 2.0f, shadowW);
    }

    // --- Aluminium border 1px ---
    g.setColour (juce::Colour (SR::Colour::aluminium));
    g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.0f);

    // --- Tick marks around outside ---
    // Drawn at slightly larger radius than the knob border
    {
        const float tickOuter = r + 5.0f;
        const float tickInner = r + 2.0f;
        const juce::Colour tickCol = juce::Colour (SR::Colour::aluminium).withAlpha (0.68f);
        g.setColour (tickCol);

        const float sweep = kEndAngle - kStartAngle;
        const int numTicks = (int) (sweep / juce::degreesToRadians (9.0f));
        for (int i = 0; i <= numTicks; ++i)
        {
            float angle = kStartAngle + (float) i * juce::degreesToRadians (9.0f);
            float sinA = std::sin (angle);
            float cosA = std::cos (angle);
            float x1 = cx + sinA * tickInner;
            float y1 = cy - cosA * tickInner;
            float x2 = cx + sinA * tickOuter;
            float y2 = cy - cosA * tickOuter;
            g.drawLine (x1, y1, x2, y2, 1.0f);
        }
    }

    // --- Indicator line: top 55% cobalt, bottom 45% ivory ---
    {
        float angle = valueToAngle (currentValue);
        float sinA  = std::sin (angle);
        float cosA  = std::cos (angle);

        // Line from center outward to 33% of radius
        float lineLen = r * 0.33f;
        float midX = cx + sinA * (lineLen * 0.55f);
        float midY = cy - cosA * (lineLen * 0.55f);
        float endX = cx + sinA * lineLen;
        float endY = cy - cosA * lineLen;

        // Cobalt portion (from center to 55%)
        g.setColour (juce::Colour (SR::Colour::accent));
        g.drawLine (cx, cy, midX, midY, 2.0f);

        // Ivory portion (from 55% to 100%)
        g.setColour (juce::Colour (SR::Colour::ink));
        g.drawLine (midX, midY, endX, endY, 2.0f);
    }

    // --- Label (9px, bold, uppercase) ---
    if (labelText.isNotEmpty())
    {
        float labelY = knobAreaH + 4.0f;
        g.setFont (SR::Font::sans (9.0f, juce::Font::bold));
        g.setColour (juce::Colour (SR::Colour::muted));
        g.drawText (labelText.toUpperCase(),
                    juce::Rectangle<float> (0, labelY, bounds.getWidth(), 13.0f),
                    juce::Justification::centred, false);
    }

    // --- Value text (10px, mono, cobalt) ---
    {
        float valY = knobAreaH + 17.0f;
        juce::String valStr = juce::String (juce::roundToInt (currentValue * 100)) + "%";
        g.setFont (SR::Font::mono (10.0f));
        g.setColour (juce::Colour (SR::Colour::accent));
        g.drawText (valStr,
                    juce::Rectangle<float> (0, valY, bounds.getWidth(), 13.0f),
                    juce::Justification::centred, false);
    }
}

void SpatialRotaryKnob::resized() {}

void SpatialRotaryKnob::setValue (float newValue, juce::NotificationType n)
{
    newValue = juce::jlimit (0.0f, 1.0f, newValue);
    if (newValue == currentValue) return;
    currentValue = newValue;
    repaint();
    if (n != juce::dontSendNotification && onValueChange)
        onValueChange (currentValue);
}

void SpatialRotaryKnob::setLabel (const juce::String& text)
{
    labelText = text;
    repaint();
}

void SpatialRotaryKnob::mouseDown (const juce::MouseEvent&)
{
    dragStartVal = currentValue;
}

void SpatialRotaryKnob::mouseDrag (const juce::MouseEvent& e)
{
    float delta = (float) e.getDistanceFromDragStartY() * -0.0055f;
    setValue (juce::jlimit (0.0f, 1.0f, dragStartVal + delta));
}

void SpatialRotaryKnob::mouseDoubleClick (const juce::MouseEvent&)
{
    setValue (defaultValue);
}

void SpatialRotaryKnob::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    setValue (juce::jlimit (0.0f, 1.0f, currentValue + w.deltaY * 0.02f));
}
