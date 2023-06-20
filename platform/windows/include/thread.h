#pragma once

#ifndef _thread_h_
#define _thread_h_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// https://learn.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2022
//
// Usage: SetThreadName (-1, "MainThread");
//
//
// Usage: SetThreadName ((DWORD)-1, "MainThread");
//
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
    DWORD dwType;     // Must be 0x1000.
    LPCSTR szName;    // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, const char* threadName);

typedef struct structTHREAD_INFO {
    DWORD id;
    DWORD key;
    char* name;
} THREAD_INFO;

#ifdef __cplusplus
}
#endif

extern DWORD selfThreadKey;

class DummyClassThread {
public:
    DummyClassThread() { selfThreadKey = TlsAlloc(); }

    ~DummyClassThread() { TlsFree(selfThreadKey); }
};

extern DummyClassThread dummyClassThread;

THREAD_INFO* GetCurrentThreadInfo();

namespace mbgl {
namespace platform {
std::string getCurrentThreadName();
void setCurrentThreadName(const std::string& name);
void makeThreadLowPriority();
void setCurrentThreadPriority(double priority);
} // namespace platform
} // namespace mbgl

#endif
