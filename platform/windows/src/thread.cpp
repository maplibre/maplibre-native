#pragma once

#include <mbgl/util/platform.hpp>
#include <mbgl/platform/thread.hpp>
#include <mbgl/util/logging.hpp>

#include <string>

#include "thread.h"

DWORD selfThreadKey;
DummyClassThread dummyClassThread;

// https://learn.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2022
void SetThreadName(DWORD dwThreadID, const char* threadName) {
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
#pragma warning(push)

#pragma warning(disable : 6320 6322)
    __try {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
#pragma warning(pop)
}

THREAD_INFO* GetCurrentThreadInfo() {
    THREAD_INFO* info = (THREAD_INFO*)TlsGetValue(selfThreadKey);

    if (!info) {
        info = (THREAD_INFO*)calloc(1, sizeof(THREAD_INFO));
        info->id = GetCurrentThreadId();
        info->key = selfThreadKey;
        info->name = NULL;

        TlsSetValue(selfThreadKey, info);
    }

    return info;
}

namespace mbgl {
namespace platform {

std::string getCurrentThreadName() {
    THREAD_INFO* info = GetCurrentThreadInfo();

    if (info && info->name) {
        return std::string(info->name);
    }

    return std::string();
}

void setCurrentThreadName(const std::string& name) {
    THREAD_INFO* info = GetCurrentThreadInfo();

    if (info && info->name) {
        free(info->name);
        info->name = new char[name.length() + 1];
        std::strcpy(info->name, name.c_str());
    }

    SetThreadName(-1, name.c_str());
}

void makeThreadLowPriority() {
    if (!SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN)) {
        Log::Warning(Event::General, "Couldn't set thread scheduling policy");
    }
}

void setCurrentThreadPriority(double priority) {
    if (!SetThreadPriority(GetCurrentThread(), int(priority))) {
        Log::Warning(Event::General, "Couldn't set thread priority");
    }
}

void attachThread() {}

void detachThread() {}

} // namespace platform
} // namespace mbgl
