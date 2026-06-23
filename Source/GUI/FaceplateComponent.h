#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "GuiTheme.h"
#include "GuiLayout.h"

class FaceplateComponent : public juce::Component
{
public:
    FaceplateComponent();
    ~FaceplateComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Sub-draw helpers
    void drawShell         (juce::Graphics&);
    void drawNoise         (juce::Graphics&);
    void drawCornerAccents (juce::Graphics&);
    void drawScrews        (juce::Graphics&);
    void drawSeparators    (juce::Graphics&);
    void drawModuleBorders (juce::Graphics&);
    void drawModuleLabels  (juce::Graphics&);
    void drawHeader        (juce::Graphics&);
    void drawFooter        (juce::Graphics&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaceplateComponent)
};
