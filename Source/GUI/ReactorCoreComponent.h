#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "GuiTheme.h"

class ReactorCoreComponent : public juce::Component
{
public:
    ReactorCoreComponent();
    ~ReactorCoreComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // t = 0..1 normalized amount
    void setAmount (float t);
    float getAmount() const { return amount; }

private:
    float amount = 0.5f;
    float cachedAmount = -1.0f;     // trigger glow cache rebuild
    juce::Image glowCache;

    void rebuildGlowCache();

    static constexpr float kBaseDiameter = 280.0f; // Component design space

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReactorCoreComponent)
};
