// seh_wrapper.c — pure C, no C++ objects, safe for __try/__except
#include <windows.h>

typedef int (*CertTestProc)();

__declspec(dllexport) int seh_run(CertTestProc fn) {
    __try {
        return fn();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return -1; // access violation caught
    }
}
