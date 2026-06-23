#pragma once

#include <juce_core/juce_core.h>

#ifdef _WIN32
 #include <windows.h>
#endif

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
       #ifdef _WIN32
        OutputDebugStringA (("[SpatialReactor] " + msg + "\n").toRawUTF8());
       #endif

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
       #ifdef _WIN32
        OutputDebugStringA ("[SpatialReactor] CrashLogger initialized\n");
       #endif
    }

    std::unique_ptr<juce::FileLogger> writer;
};

#define CRASH_LOG(msg)  CrashLogger::get().log (msg)
