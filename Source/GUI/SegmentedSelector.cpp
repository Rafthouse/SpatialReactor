#include "SegmentedSelector.h"

//==============================================================================
// Attachment Pimpl
//==============================================================================
class SegmentedSelector::Attachment::Pimpl : public juce::AudioProcessorParameter::Listener
{
public:
    Pimpl (juce::AudioProcessorValueTreeState& state,
           const juce::String& paramID,
           SegmentedSelector& sel)
        : param (state.getParameter (paramID)), selRef (sel)
    {
        if (param == nullptr) { jassertfalse; return; }

        auto range = param->getNormalisableRange();
        int idx = juce::roundToInt (range.convertFrom0to1 (param->getValue()));
        selRef.setSelectedIndex (idx, juce::dontSendNotification);

        selRef.onChange = [this] (int index)
        {
            auto range = param->getNormalisableRange();
            param->setValueNotifyingHost (range.convertTo0to1 ((float) index));
        };

        param->addListener (this);
    }

    ~Pimpl() override
    {
        selRef.onChange = nullptr;
        if (param != nullptr) param->removeListener (this);
    }

    void parameterValueChanged (int, float newValue) override
    {
        auto range = param->getNormalisableRange();
        int idx = juce::roundToInt (range.convertFrom0to1 (newValue));
        juce::MessageManager::callAsync ([&sel = selRef, idx]()
        {
            sel.setSelectedIndex (idx, juce::dontSendNotification);
        });
    }

    void parameterGestureChanged (int, bool) override {}

private:
    juce::RangedAudioParameter* param;
    SegmentedSelector& selRef;
};

SegmentedSelector::Attachment::Attachment (juce::AudioProcessorValueTreeState& state,
                                            const juce::String& paramID,
                                            SegmentedSelector& sel)
    : pimpl (std::make_unique<Pimpl> (state, paramID, sel)) {}

SegmentedSelector::Attachment::~Attachment() = default;

//==============================================================================
// SegmentedSelector
//==============================================================================
SegmentedSelector::SegmentedSelector (const std::vector<juce::String>& labels)
    : segmentLabels (labels)
{
    setMouseClickGrabsKeyboardFocus (false);
}

SegmentedSelector::~SegmentedSelector() = default;

juce::Rectangle<float> SegmentedSelector::getSegmentBounds (int index) const
{
    auto area = getLocalBounds().toFloat();
    int n = (int) segmentLabels.size();
    if (n == 0) return {};
    float w = area.getWidth() / (float) n;
    return { area.getX() + (float) index * w, area.getY(), w, area.getHeight() };
}

void SegmentedSelector::setSelectedIndex (int index, juce::NotificationType n)
{
    int clamped = juce::jlimit (0, (int) segmentLabels.size() - 1, index);
    if (clamped == selectedIndex) return;
    selectedIndex = clamped;
    repaint();
    if (n != juce::dontSendNotification && onChange)
        onChange (selectedIndex);
}

void SegmentedSelector::mouseDown (const juce::MouseEvent& e)
{
    int n = (int) segmentLabels.size();
    for (int i = 0; i < n; ++i)
    {
        if (getSegmentBounds (i).toNearestInt().contains (e.getMouseDownPosition()))
        {
            setSelectedIndex (i);
            break;
        }
    }
}

void SegmentedSelector::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    int  n    = (int) segmentLabels.size();
    if (n == 0) return;

    // --- Background ---
    g.setColour (juce::Colour (0x00000000).withAlpha (0.14f));
    g.fillRect (area);

    // --- Outer border ---
    g.setColour (SR::Colour::line());
    g.drawRect (area, 1.0f);

    // --- Segments ---
    float segW = area.getWidth() / (float) n;
    for (int i = 0; i < n; ++i)
    {
        auto segBounds = getSegmentBounds (i);
        bool isActive  = (i == selectedIndex);

        // Active background
        if (isActive)
        {
            g.setColour (SR::Colour::accentSoft());
            g.fillRect (segBounds);

            // Cobalt bottom underline (2px)
            auto underline = segBounds.removeFromBottom (2.0f);
            g.setColour (juce::Colour (SR::Colour::accent));
            g.fillRect (underline);
        }

        // Vertical separator (except before first)
        if (i > 0)
        {
            g.setColour (SR::Colour::line());
            g.drawLine (segBounds.getX(), segBounds.getY(), segBounds.getX(), segBounds.getBottom(), 1.0f);
        }

        // Label
        juce::String lbl = segmentLabels[(size_t) i].toUpperCase();
        g.setFont (SR::Font::sans (8.0f, juce::Font::bold));
        g.setColour (isActive ? juce::Colour (SR::Colour::ink) : juce::Colour (SR::Colour::muted));
        g.drawText (lbl, segBounds.toNearestInt(), juce::Justification::centred, false);
    }
}

void SegmentedSelector::resized() {}
