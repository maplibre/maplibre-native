/*

 *
 */

#pragma once

#include "JniReference.h"
#include <string_view>

namespace glue_internal {
namespace jni {

class JNIEXPORT CachedJavaClassBase {
public:
    explicit CachedJavaClassBase(const char* name);
    CachedJavaClassBase(const char* name, const char* cpp_name);
    virtual ~CachedJavaClassBase();

    static void init(JNIEnv* env);

protected:
    static jni::JniReference<jclass>* get_java_class_for_instance(const std::string_view& id);

private:
    void do_init(JNIEnv* env);
    virtual JniReference<jclass>* set_java_class(JniReference<jclass> java_class_reference) = 0;

    const char* const m_name;
    const char* const m_cpp_name;
};

template <class... CppTypes>
class CachedJavaClass final : public CachedJavaClassBase {
public:
    using CachedJavaClassBase::CachedJavaClassBase;
    static const jni::JniReference<jclass>& get_java_class(const std::string_view& id) {
        auto instance_java_class = get_java_class_for_instance(id);
        return instance_java_class != nullptr ? *instance_java_class : java_class;
    }

    static JniReference<jclass> java_class;

private:
    JniReference<jclass>* set_java_class(JniReference<jclass> java_class_reference) override {
        java_class = std::move(java_class_reference);
        return &java_class;
    }
};

template <class... CppTypes>
class CachedJavaInterface final : public CachedJavaClassBase {
public:
    using CachedJavaClassBase::CachedJavaClassBase;

    static JniReference<jclass> java_class;

private:
    JniReference<jclass>* set_java_class(JniReference<jclass> java_class_reference) override {
        java_class = std::move(java_class_reference);
        return &java_class;
    }
};

template <class... CppTypes>
JniReference<jclass> CachedJavaClass<CppTypes...>::java_class;

template <class... CppTypes>
JniReference<jclass> CachedJavaInterface<CppTypes...>::java_class;

#define REGISTER_JNI_CLASS_CACHE(name, register_name, ...) \
    namespace {                                            \
    CachedJavaClass<__VA_ARGS__> register_name(name);      \
    }

#define REGISTER_JNI_CLASS_CACHE_INHERITANCE(name, register_name, cpp_name, ...) \
    namespace {                                                                  \
    CachedJavaClass<__VA_ARGS__> register_name(name, cpp_name);                  \
    }

#define REGISTER_JNI_CLASS_CACHE_INTERFACE(name, register_name, ...) \
    namespace {                                                      \
    CachedJavaInterface<__VA_ARGS__> register_name(name);            \
    }

JNIEXPORT JniReference<jclass>& get_cached_native_base_class();
JNIEXPORT JniReference<jclass>& get_cached_duration_class();

} // namespace jni

} // namespace glue_internal
