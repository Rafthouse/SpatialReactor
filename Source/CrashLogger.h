#pragma once

#include <juce_core/juce_core.h>
#include <windows.h>

/**
    Dual-mode crash logger:
    1. OutputDebugString → visible in DebugView real-time
    2. FileLogger → persistent log file
*/
class CrashLogger
{
public:
    static CrashLogger& get()
    {
        static CrashLogger instance;
        return instance;
    }

    void log (const juce::String& msg)
    {
        // Real-time: visible in DebugView immediately
        OutputDebugStringA (("[SpatialReactor] " + msg + "\n").toRawUTF8());

        // Persistent: written to file
        if (writer != nullptr)
            writer->logMessage (msg);
    }

private:
    CrashLogger()
    {
        auto logFile = juce::File::getSpecialLocation (juce::File::tempDirectory)
                           .getChildFile ("SpatialReactor_crash.log");
        logFile.deleteFile();
        writer = std::make_unique<juce::FileLogger> (logFile, "SpatialReactor");
        // Also write to DebugView immediately
        OutputDebugStringA ("[SpatialReactor] CrashLogger initialized\n");
    }

    std::unique_ptr<juce::FileLogger> writer;
};

#define CRASH_LOG(msg)  CrashLogger::get().log (msg)
