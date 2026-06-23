#include "BauhausSafeFreeToggle.h"

//==============================================================================
class BauhausSafeFreeToggle::Attachment::Pimpl : public juce::AudioProcessorParameter::Listener
{
public:
    Pimpl (juce::AudioProcessorValueTreeState& state,
           const juce::String& paramID,
           BauhausSafeFreeToggle& toggle)
        : param (state.getParameter (paramID)),
          toggleRef (toggle)
    {
        if (param == nullptr) { jassertfalse; return; }

        // Initial state
        toggleRef.setState (param->getValue() > 0.5f, juce::dontSendNotification);

        // Toggle → param
        toggleRef.onToggle = [this] (bool safe)
        {
            param->setValueNotifyingHost (safe ? 1.0f : 0.0f);
        };

        // Param → toggle
        param->addListener (this);
    }

    ~Pimpl()
    {
        toggleRef.onToggle = nullptr;
        if (param != nullptr) param->removeListener (this);
    }

    void parameterValueChanged (int, float newValue) override
    {
        // Capture toggle reference directly — outlives Pimpl
        juce::MessageManager::callAsync ([&toggle = toggleRef, newValue]()
        {
            toggle.setState (newValue > 0.5f, juce::dontSendNotification);
        });
    }

    void parameterGestureChanged (int, bool) override {}

private:
    juce::RangedAudioParameter* param;
    BauhausSafeFreeToggle& toggleRef;
};

//==============================================================================
BauhausSafeFreeToggle::Attachment::Attachment (juce::AudioProcessorValueTreeState& state,
                                                const juce::String& paramID,
                                                BauhausSafeFreeToggle& toggle)
    : pimpl (std::make_unique<Pimpl> (state, paramID, toggle)) {}

BauhausSafeFreeToggle::Attachment::~Attachment() = default;

//==============================================================================
BauhausSafeFreeToggle::BauhausSafeFreeToggle()
{
    setMouseClickGrabsKeyboardFocus (false);
}

BauhausSafeFreeToggle::~BauhausSafeFreeToggle() = default;

void BauhausSafeFreeToggle::setState (bool state, juce::NotificationType notification)
{
    if (state != safeMode)
    {
        safeMode = state;
        repaint();

        if (notification != juce::dontSendNotification && onToggle)
            onToggle (safeMode);
    }
}

void BauhausSafeFreeToggle::mouseDown (const juce::MouseEvent&)
{
    setState (!safeMode);
}

void BauhausSafeFreeToggle::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    auto halfW = area.getWidth() * 0.5f;

    // -- Track background --
    g.setColour (juce::Colour (0xff2C2E31));
    g.fillRoundedRectangle (area, 3.0f);

    // -- Track border --
    g.setColour (juce::Colour (0xffA8ADB3));
    g.drawRoundedRectangle (area, 3.0f, 1.5f);

    // -- Active thumb --
    float thumbLeft = safeMode ? 0.0f : halfW;
    auto thumbR = area.withWidth (halfW).withX (thumbLeft);
    g.setColour (juce::Colour (0xff2D6BFF));
    g.fillRoundedRectangle (thumbR, 3.0f);

    // -- Labels --
    auto leftR  = area.withWidth (halfW);
    auto rightR = area.withWidth (halfW).withX (halfW);

    // SAFE
    g.setColour (safeMode ? juce::Colours::white : juce::Colour (0x59E8E6E3));
    g.setFont (juce::Font (area.getHeight() * 0.42f, juce::Font::bold));
    g.drawText ("SAFE", leftR.toNearestInt(), juce::Justification::centred, 1);

    // FREE
    g.setColour (!safeMode ? juce::Colours::white : juce::Colour (0x59E8E6E3));
    g.drawText ("FREE", rightR.toNearestInt(), juce::Justification::centred, 1);
}

void BauhausSafeFreeToggle::resized()
{
}
