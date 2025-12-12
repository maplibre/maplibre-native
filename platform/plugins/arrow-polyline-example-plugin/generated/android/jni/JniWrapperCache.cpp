/*

 *
 */

#include "JniWrapperCache.h"
#include "JniThrowNewException.h"

#include <jni.h>

#include <mutex>
#include <unordered_map>

namespace glue_internal {
namespace jni {

struct CachedObject {
    jobject obj = nullptr;
    unsigned int scheduled_removals = 0;
};

static std::mutex s_mutex;
static std::unordered_map<const void*, CachedObject> s_wrapper_cache;

void JniWrapperCache::cache_wrapper_impl(JNIEnv* jenv, const void* obj_ptr, const JniReference<jobject>& jobj) {
    std::lock_guard<std::mutex> lock(s_mutex);

#ifdef GLUECODIUM_ENABLE_INTERNAL_DEBUG_CHECKS
    auto iter = s_wrapper_cache.find(obj_ptr);
    if (iter != s_wrapper_cache.end() && iter->second.obj != nullptr) {
        throw_new_runtime_exception(jenv,
                                    "Weak reference leaked! A possible root cause can be calling platform "
                                    "method with 'this' argument from C++ factory function.\nPlease check "
                                    "'@AfterConstructed()' attribute from the following link:\n"
                                    "\thttps://github.com/heremaps/gluecodium/blob/master/docs/lime_attributes.md");
    }
#endif

    s_wrapper_cache[obj_ptr].obj = jenv->NewWeakGlobalRef(jobj.get());
}

JniReference<jobject> JniWrapperCache::get_cached_wrapper_impl(JNIEnv* jenv, const void* obj_ptr) {
    std::lock_guard<std::mutex> lock(s_mutex);

    auto iter = s_wrapper_cache.find(obj_ptr);
    if (iter == s_wrapper_cache.end()) return {};

    if (iter->second.obj == nullptr) {
        return {};
    }

    auto jobj = jenv->NewLocalRef(iter->second.obj);
    if (jenv->IsSameObject(jobj, NULL)) {
        jenv->DeleteLocalRef(jobj);

        // Object reference has been cleared by garbage collector.
        // To avoid leaks we need to delete the weak ref object here.
        jenv->DeleteWeakGlobalRef(iter->second.obj);
        iter->second.obj = nullptr;

        // However, because we "remove" the entry from cache here we need to ensure that
        // the cleanup, which will be scheduled for already cleared object (which has not
        // been performed yet) does not touch any cache entry added later (an entry for new object
        // will be added by 'convert_to_jni()').
        iter->second.scheduled_removals++;

        return {};
    } else {
        return make_local_ref(jenv, jobj);
    }
}

void JniWrapperCache::remove_cached_wrapper_impl(JNIEnv* jenv, const void* obj_ptr) {
    std::lock_guard<std::mutex> lock(s_mutex);

    auto iter = s_wrapper_cache.find(obj_ptr);

#ifdef GLUECODIUM_ENABLE_INTERNAL_DEBUG_CHECKS
    if (iter == s_wrapper_cache.end()) {
        throw_new_runtime_exception(jenv,
                                    "Invalid removal of cache entry. A possible root cause can be calling platform "
                                    "method with 'this' argument from C++ factory function.\nPlease check "
                                    "'@AfterConstructed()' attribute from the following link:\n"
                                    "\thttps://github.com/heremaps/gluecodium/blob/master/docs/lime_attributes.md");
    }
#endif

    if (iter != s_wrapper_cache.end()) {
        // Firstly, verify if there were multiple cleanups scheduled for the given entry.
        // This situation may happen if the exceptional removal of entry happened in
        // JniWrapperCache::get_cached_wrapper_impl() when the weak reference was cleared by
        // garbage collector. In such a case does not touch the entry - the last scheduled
        // cleanup will remove it.
        if (iter->second.scheduled_removals > 0) {
            iter->second.scheduled_removals--;
            return;
        }

        if (iter->second.obj != nullptr) {
            jenv->DeleteWeakGlobalRef(iter->second.obj);
        }
        s_wrapper_cache.erase(iter);
    }
}

} // namespace jni
} // namespace glue_internal
