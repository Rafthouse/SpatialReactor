#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "GuiTheme.h"

class SegmentedSelector : public juce::Component
{
public:
    explicit SegmentedSelector (const std::vector<juce::String>& labels);
    ~SegmentedSelector() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

    void setSelectedIndex (int index, juce::NotificationType n = juce::sendNotification);
    int  getSelectedIndex() const { return selectedIndex; }

    std::function<void (int)> onChange;

    // APVTS attachment for Choice parameter
    class Attachment
    {
    public:
        Attachment (juce::AudioProcessorValueTreeState&, const juce::String& paramID, SegmentedSelector&);
        ~Attachment();
    private:
        class Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

private:
    std::vector<juce::String> segmentLabels;
    int selectedIndex = 0;

    juce::Rectangle<float> getSegmentBounds (int index) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SegmentedSelector)
};
