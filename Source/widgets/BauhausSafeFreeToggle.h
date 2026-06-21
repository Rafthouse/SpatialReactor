#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
    A two-segment toggle switch with labels SAFE / FREE.
    Left half = SAFE (blue when active), Right half = FREE.
    Supports APVTS attachment via ButtonAttachment.
*/
class BauhausSafeFreeToggle : public juce::Component
{
public:
    //==============================================================================
    BauhausSafeFreeToggle();
    ~BauhausSafeFreeToggle() override;

    /** Set toggle state. true = SAFE (left), false = FREE (right). */
    void setState (bool safeMode, juce::NotificationType notification = juce::sendNotification);
    bool getState() const noexcept { return safeMode; }

    /** Callback when toggled. */
    std::function<void(bool)> onToggle;

    //==============================================================================
    /** APVTS attachment. */
    class Attachment
    {
    public:
        Attachment (juce::AudioProcessorValueTreeState& state,
                    const juce::String& paramID,
                    BauhausSafeFreeToggle& toggle);
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
    bool safeMode = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BauhausSafeFreeToggle)
};
