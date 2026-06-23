#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "GuiTheme.h"

class TrustToggle : public juce::Component
{
public:
    TrustToggle();
    ~TrustToggle() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

    // true = SAFE, false = FREE
    void setState (bool safe, juce::NotificationType n = juce::sendNotification);
    bool getState() const { return safeMode; }

    std::function<void (bool)> onToggle;

    // APVTS attachment for bool parameter
    class Attachment
    {
    public:
        Attachment (juce::AudioProcessorValueTreeState&, const juce::String& paramID, TrustToggle&);
        ~Attachment();
    private:
        class Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

private:
    bool safeMode = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrustToggle)
};
