#include <cstdio>
#include <juce_core/juce_core.h>

class EmptyApp : public juce::JUCEApplication
{
public:
    EmptyApp() { printf("EmptyApp constructor\n"); fflush(stdout); }
    const juce::String getApplicationName() override { return "Empty"; }
    const juce::String getApplicationVersion() override { return "1.0"; }

    void initialise (const juce::String&) override
    {
        printf("EmptyApp initialise — OK, JUCE is alive\n");
        fflush(stdout);
        signalAppTerminated = true;
    }
    void shutdown() override {}

    static bool signalAppTerminated;
};
bool EmptyApp::signalAppTerminated = false;

START_JUCE_APPLICATION (EmptyApp)
