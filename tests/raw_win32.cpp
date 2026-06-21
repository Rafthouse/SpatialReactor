#include <cstdio>
#include <windows.h>

// No JUCEApplication, no message loop, no static initializers.
// Just test if a simple Win32 console app can even START.
int main()
{
    printf("=== RAW WIN32 TEST ===\n");
    fflush(stdout);

    HMODULE self = GetModuleHandleA(NULL);
    if (self == NULL) {
        printf("GetModuleHandle failed\n");
        fflush(stdout);
        return 1;
    }
    printf("Module handle: %p\n", (void*)self);
    fflush(stdout);

    // Try to load a JUCE symbol to verify JUCE is linked
    FARPROC proc = GetProcAddress(self, "CreatePluginFilter");
    if (proc == NULL) {
        printf("CreatePluginFilter not exported (expected for console test)\n");
        fflush(stdout);
    } else {
        printf("CreatePluginFilter found at: %p\n", (void*)proc);
        fflush(stdout);
    }

    printf("=== WIN32 OK ===\n");
    fflush(stdout);
    return 0;
}
