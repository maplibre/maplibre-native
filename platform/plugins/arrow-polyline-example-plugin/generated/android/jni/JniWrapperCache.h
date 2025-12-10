/*

 *
 */

#pragma once

#include "JniReference.h"

namespace glue_internal
{
namespace jni
{

class JNIEXPORT JniWrapperCache final
{
public:
    template<class T>
    static void cache_wrapper(JNIEnv* jenv, const std::shared_ptr<T>& nobj, const JniReference<jobject>& jobj) {
        cache_wrapper_impl(jenv, nobj.get(), jobj);
    }

    template<class T>
    static JniReference<jobject> get_cached_wrapper(JNIEnv* jenv, const std::shared_ptr<T>& nobj) {
        return get_cached_wrapper_impl(jenv, nobj.get());
    }

    template<class T>
    static void remove_cached_wrapper(JNIEnv* jenv, const std::shared_ptr<T>& nobj) {
        remove_cached_wrapper_impl(jenv, nobj.get());
    }

private:
    static void cache_wrapper_impl(JNIEnv* jenv, const void* obj_ptr, const JniReference<jobject>& jobj);
    static JniReference<jobject> get_cached_wrapper_impl(JNIEnv* jenv, const void* obj_ptr);
    static void remove_cached_wrapper_impl(JNIEnv* jenv, const void* obj_ptr);
};

} // namespace jni
} // namespace glue_internal
