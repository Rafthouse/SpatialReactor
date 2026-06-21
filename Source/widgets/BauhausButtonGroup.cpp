#include "BauhausButtonGroup.h"

//==============================================================================
class BauhausButtonGroup::Attachment::Pimpl : public juce::AudioProcessorParameter::Listener
{
public:
    Pimpl (juce::AudioProcessorValueTreeState& state,
           const juce::String& paramID,
           BauhausButtonGroup& group)
        : param (state.getParameter (paramID)),
          groupRef (group)
    {
        if (param == nullptr)
        {
            jassertfalse;
            return;
        }

        // Initialise from parameter
        auto norm = param->getValue();
        auto range = param->getNormalisableRange();
        auto raw = range.convertFrom0to1 (norm);
        int idx = juce::roundToInt (raw);
        group.setSelectedIndex (idx, juce::dontSendNotification);

        // Listen for changes
        group.onChange = [this] (int index)
        {
            auto range = param->getNormalisableRange();
            auto norm = range.convertTo0to1 ((float) index);
            param->setValueNotifyingHost (norm);
        };

        // Listen for external parameter changes
        param->addListener (this);
    }

    ~Pimpl()
    {
        groupRef.onChange = nullptr;
        if (param != nullptr)
            param->removeListener (this);
    }

    void parameterValueChanged (int /*paramIndex*/, float newValue) override
    {
        auto range = param->getNormalisableRange();
        auto raw = range.convertFrom0to1 (newValue);
        int idx = juce::roundToInt (raw);
        // Capture group reference directly — outlives Pimpl
        juce::MessageManager::callAsync ([&group = groupRef, idx]()
        {
            group.setSelectedIndex (idx, juce::dontSendNotification);
        });
    }

    void parameterGestureChanged (int, bool) override {}

private:
    juce::RangedAudioParameter* param;
    BauhausButtonGroup& groupRef;
};

//==============================================================================
BauhausButtonGroup::Attachment::Attachment (juce::AudioProcessorValueTreeState& state,
                                            const juce::String& paramID,
                                            BauhausButtonGroup& group)
    : pimpl (std::make_unique<Pimpl> (state, paramID, group)) {}

BauhausButtonGroup::Attachment::~Attachment() = default;

//==============================================================================
BauhausButtonGroup::BauhausButtonGroup()  { setMouseClickGrabsKeyboardFocus (false); }

BauhausButtonGroup::BauhausButtonGroup (const std::vector<juce::String>& labels)
{
    setMouseClickGrabsKeyboardFocus (false);
    juce::Array<Item> arr;
    for (int i = 0; i < (int) labels.size(); ++i)
        arr.add ({ labels[i], i });
    init ("", arr, 0);
}

BauhausButtonGroup::~BauhausButtonGroup() = default;

void BauhausButtonGroup::init (const juce::String& label,
                                const juce::Array<Item>& newItems,
                                int defaultIndex)
{
    groupLabel = label;
    items = newItems;
    selectedIndex = juce::jlimit (0, items.size() - 1, defaultIndex);
    resized();
    repaint();
}

void BauhausButtonGroup::setSelectedIndex (int index, juce::NotificationType notification)
{
    index = juce::jlimit (0, items.size() - 1, index);
    if (index != selectedIndex)
    {
        selectedIndex = index;
        repaint();

        if (notification != juce::dontSendNotification && onChange)
            onChange (selectedIndex);
    }
}

void BauhausButtonGroup::mouseDown (const juce::MouseEvent& e)
{
    for (int i = 0; i < items.size(); ++i)
    {
        if (getItemBounds (i).contains (e.getMouseDownPosition()))
        {
            setSelectedIndex (i);
            break;
        }
    }
}

juce::Rectangle<int> BauhausButtonGroup::getItemBounds (int index) const
{
    auto area = getLocalBounds();
    // Reserve top for group label
    area.removeFromTop (14);

    auto itemHeight = ledGap;
    auto y = area.getY() + index * itemHeight;
    return { 0, y, getWidth(), itemHeight };
}

void BauhausButtonGroup::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    // Group label
    if (groupLabel.isNotEmpty())
    {
        g.setColour (juce::Colour (0x3FE8E6E3)); // 25% white
        g.setFont (juce::Font (7.0f, juce::Font::bold));
        g.drawText (groupLabel, area.removeFromTop (12), juce::Justification::centredLeft);
    }

    // Items
    for (int i = 0; i < items.size(); ++i)
    {
        auto r = getItemBounds (i);

        // LED circle
        auto ledBounds = r.removeFromLeft (ledSize + 4).withSizeKeepingCentre (ledSize, ledSize);

        bool isActive = (i == selectedIndex);
        bool isHover  = (i == hoveredIndex);

        // Outer ring
        g.setColour (juce::Colour (0xffA8ADB3).withAlpha (isActive ? 1.0f : 0.3f));
        g.drawEllipse (ledBounds.toFloat(), 1.5f);

        // Inner fill
        if (isActive)
        {
            g.setColour (juce::Colour (0xff2D6BFF));
            g.fillEllipse (ledBounds.reduced (3).toFloat());
        }
        else if (isHover)
        {
            g.setColour (juce::Colour (0xff2D6BFF).withAlpha (0.15f));
            g.fillEllipse (ledBounds.reduced (3).toFloat());
        }

        // Label text
        g.setColour (isActive ? juce::Colour (0xffE8E6E3) : juce::Colour (0x59E8E6E3));
        g.setFont (juce::Font (10.0f, juce::Font::bold));
        g.drawText (items[i].label, r, juce::Justification::centredLeft);
    }
}

void BauhausButtonGroup::resized()
{
    int totalHeight = 14 + items.size() * ledGap;
    setSize (getWidth(), totalHeight);
}
