#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>

//==============================================================================
/**
    A vertical radio-button group with LED indicators.
    Each item is a circle (LED) + label, arranged vertically.
    Supports APVTS attachment via custom attachment class.
*/
class BauhausButtonGroup : public juce::Component
{
public:
    //==============================================================================
    struct Item
    {
        juce::String label;
        int          value = 0;
    };

    //==============================================================================
    BauhausButtonGroup();
    BauhausButtonGroup (const std::vector<juce::String>& labels);
    ~BauhausButtonGroup() override;

    /** Initialise the group with items and a label for the group itself. */
    void init (const juce::String& groupLabel,
               const juce::Array<Item>& items,
               int defaultIndex = 0);

    /** Set the selected index programmatically. */
    void setSelectedIndex (int index, juce::NotificationType notification = juce::sendNotification);

    /** Get the currently selected index. */
    int getSelectedIndex() const noexcept { return selectedIndex; }

    /** Callback fired when selection changes. */
    std::function<void(int)> onChange;

    //==============================================================================
    /** Custom attachment to link a float parameter to a button group. */
    class Attachment
    {
    public:
        Attachment (juce::AudioProcessorValueTreeState& state,
                    const juce::String& paramID,
                    BauhausButtonGroup& group);
        ~Attachment();

    private:
        class Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void mouseDown (const juce::MouseEvent&) override;

private:
    //==============================================================================
    juce::String groupLabel;
    juce::Array<Item> items;
    int selectedIndex = 0;
    int hoveredIndex  = -1;

    // Layout cache
    juce::Rectangle<int> getItemBounds (int index) const;
    int ledSize = 12;
    int ledGap  = 28;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BauhausButtonGroup)
};
