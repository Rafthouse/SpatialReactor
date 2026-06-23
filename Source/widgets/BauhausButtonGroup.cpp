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

        auto norm = param->getValue();
        auto range = param->getNormalisableRange();
        auto raw = range.convertFrom0to1 (norm);
        int idx = juce::roundToInt (raw);
        group.setSelectedIndex (idx, juce::dontSendNotification);

        group.onChange = [this] (int index)
        {
            auto range = param->getNormalisableRange();
            auto norm = range.convertTo0to1 ((float) index);
            param->setValueNotifyingHost (norm);
        };

        param->addListener (this);
    }

    ~Pimpl()
    {
        groupRef.onChange = nullptr;
        if (param != nullptr)
            param->removeListener (this);
    }

    void parameterValueChanged (int, float newValue) override
    {
        auto range = param->getNormalisableRange();
        auto raw = range.convertFrom0to1 (newValue);
        int idx = juce::roundToInt (raw);
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
    int numItems = items.size();
    if (numItems == 0) return {};

    int itemWidth = area.getWidth() / numItems;
    return { area.getX() + index * itemWidth, area.getY(), itemWidth, area.getHeight() };
}

void BauhausButtonGroup::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    // Background track
    g.setColour (juce::Colour (0xff2C2E31));
    g.fillRoundedRectangle (area, 4.0f);
    g.setColour (juce::Colour (0xffA8ADB3).withAlpha (0.2f));
    g.drawRoundedRectangle (area, 4.0f, 1.0f);

    int numItems = items.size();
    if (numItems == 0) return;

    // Active segment highlight
    float segW = area.getWidth() / (float) numItems;
    auto activeRect = area.withWidth (segW).withX (area.getX() + selectedIndex * segW);
    g.setColour (juce::Colour (0xff2D6BFF));
    g.fillRoundedRectangle (activeRect.reduced (2.0f, 2.0f), 3.0f);

    // Labels
    for (int i = 0; i < numItems; ++i)
    {
        auto segRect = area.withWidth (segW).withX (area.getX() + i * segW);
        bool isActive = (i == selectedIndex);

        g.setColour (isActive ? juce::Colours::white : juce::Colour (0x99E8E6E3));
        g.setFont (juce::Font (10.0f, juce::Font::bold));
        g.drawText (items[i].label, segRect.toNearestInt(), juce::Justification::centred, false);
    }
}

void BauhausButtonGroup::resized()
{
    // Horizontal layout — respect parent's setBounds, don't override
}
