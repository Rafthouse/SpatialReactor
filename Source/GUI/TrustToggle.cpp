#include "TrustToggle.h"

//==============================================================================
// Attachment Pimpl
//==============================================================================
class TrustToggle::Attachment::Pimpl : public juce::AudioProcessorParameter::Listener
{
public:
    Pimpl (juce::AudioProcessorValueTreeState& state,
           const juce::String& paramID,
           TrustToggle& toggle)
        : param (state.getParameter (paramID)), toggleRef (toggle)
    {
        if (param == nullptr) { jassertfalse; return; }
        toggleRef.setState (param->getValue() > 0.5f, juce::dontSendNotification);
        toggleRef.onToggle = [this] (bool safe) { param->setValueNotifyingHost (safe ? 1.0f : 0.0f); };
        param->addListener (this);
    }

    ~Pimpl() override
    {
        toggleRef.onToggle = nullptr;
        if (param != nullptr) param->removeListener (this);
    }

    void parameterValueChanged (int, float newValue) override
    {
        juce::MessageManager::callAsync ([&toggle = toggleRef, newValue]()
        {
            toggle.setState (newValue > 0.5f, juce::dontSendNotification);
        });
    }

    void parameterGestureChanged (int, bool) override {}

private:
    juce::RangedAudioParameter* param;
    TrustToggle& toggleRef;
};

TrustToggle::Attachment::Attachment (juce::AudioProcessorValueTreeState& state,
                                      const juce::String& paramID,
                                      TrustToggle& toggle)
    : pimpl (std::make_unique<Pimpl> (state, paramID, toggle)) {}

TrustToggle::Attachment::~Attachment() = default;

//==============================================================================
// TrustToggle
//==============================================================================
TrustToggle::TrustToggle()
{
    setMouseClickGrabsKeyboardFocus (false);
}

TrustToggle::~TrustToggle() = default;

void TrustToggle::setState (bool safe, juce::NotificationType n)
{
    if (safe == safeMode) return;
    safeMode = safe;
    repaint();
    if (n != juce::dontSendNotification && onToggle)
        onToggle (safeMode);
}

void TrustToggle::mouseDown (const juce::MouseEvent&)
{
    setState (!safeMode);
}

void TrustToggle::paint (juce::Graphics& g)
{
    auto area  = getLocalBounds().toFloat();
    float halfW = area.getWidth() * 0.5f;

    // --- Dark base ---
    g.setColour (juce::Colour (SR::Colour::background));
    g.fillRect (area);

    // --- Outer border ---
    g.setColour (SR::Colour::line());
    g.drawRect (area, 1.0f);

    // --- Active half: accent-soft fill + 2px cobalt bottom border ---
    auto safeHalf = area.withWidth (halfW);
    auto freeHalf = area.withX (area.getX() + halfW).withWidth (halfW);

    auto activeHalf = safeMode ? safeHalf : freeHalf;

    g.setColour (SR::Colour::accentSoft());
    g.fillRect (activeHalf);

    // Cobalt bottom border on active half
    g.setColour (juce::Colour (SR::Colour::accent));
    g.fillRect (activeHalf.removeFromBottom (2.0f));

    // Vertical divider
    g.setColour (SR::Colour::line());
    g.drawLine (area.getX() + halfW, area.getY(), area.getX() + halfW, area.getBottom(), 1.0f);

    // --- Labels ---
    g.setFont (SR::Font::sans (8.0f, juce::Font::bold));

    // SAFE
    g.setColour (safeMode ? juce::Colour (SR::Colour::ink) : juce::Colour (SR::Colour::muted));
    g.drawText ("SAFE", safeHalf.toNearestInt(), juce::Justification::centred, false);

    // FREE
    g.setColour (!safeMode ? juce::Colour (SR::Colour::ink) : juce::Colour (SR::Colour::muted));
    g.drawText ("FREE", freeHalf.toNearestInt(), juce::Justification::centred, false);
}

void TrustToggle::resized() {}
