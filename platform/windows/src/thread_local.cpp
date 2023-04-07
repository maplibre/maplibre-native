#pragma once

#include <mbgl/util/thread_local.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/platform/thread.hpp>

#include <cassert>
#include <cstdlib>

#include "thread.h"

#define StorageToThreadInfo reinterpret_cast<THREAD_INFO*&>(storage)

namespace mbgl {
namespace util {
namespace impl {

ThreadLocalBase::ThreadLocalBase() {
    static_assert(sizeof(storage) >= sizeof(THREAD_INFO*), "storage is too small");
    static_assert(alignof(decltype(storage)) % alignof(THREAD_INFO*) == 0, "storage is incorrectly aligned");

    THREAD_INFO* info = GetCurrentThreadInfo();
    THREAD_INFO* threadLocalInfo = (THREAD_INFO*)calloc(1, sizeof(THREAD_INFO));
    threadLocalInfo->id = info->id;
    threadLocalInfo->name = info->name;

    if (threadLocalInfo == NULL || (threadLocalInfo->key = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
        Log::Error(Event::General, "Failed to initialize thread-specific storage key");
        abort();
    }

    StorageToThreadInfo = threadLocalInfo;
}

ThreadLocalBase::~ThreadLocalBase() {
    // ThreadLocal will not take ownership of the pointer it is managing. The pointer
    // needs to be explicitly cleared before we destroy this object.
    assert(!get());

    DWORD key = StorageToThreadInfo->key;

    free(StorageToThreadInfo);

    if (!TlsFree(key)) {
        Log::Error(Event::General, "Failed to delete thread-specific storage key");
        abort();
    }
}

void* ThreadLocalBase::get() {
    return TlsGetValue(StorageToThreadInfo->key);
}

void ThreadLocalBase::set(void* ptr) {
    if (!TlsSetValue(StorageToThreadInfo->key, ptr)) {
        Log::Error(Event::General, "Failed to set thread-specific storage");
        abort();
    }
}

} // namespace impl
} // namespace util
} // namespace mbgl
