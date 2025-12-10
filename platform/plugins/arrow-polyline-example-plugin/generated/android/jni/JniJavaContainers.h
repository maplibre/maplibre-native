/*

 *
 */

#pragma once

#include "JniReference.h"

#include <jni.h>

#include <utility>

namespace glue_internal
{
namespace jni
{

struct JNIEXPORT JavaContainer
{
    JavaContainer(JNIEnv* const env, const char* class_name) noexcept;
    JavaContainer(JNIEnv* const env, JniReference<jclass> container_class, JniReference<jobject> instance) noexcept;

    JNIEnv* const env;
    const JniReference<jclass> container_class;
    JniReference<jobject> instance;
};

class JNIEXPORT JavaContainerAdder
{
protected:
    JavaContainerAdder(JNIEnv* const env,
                       const char* class_name) noexcept;
    JavaContainerAdder(JNIEnv* const env,
                       JniReference<jclass> container_class,
                       JniReference<jobject> instance) noexcept;

public:
    void add(const JniReference<jobject>& item) noexcept;
    JniReference<jobject> fetch_container() noexcept
    {
        return std::move(m_java_container.instance);
    }

private:
    JavaContainer m_java_container;
    const jmethodID m_add_method_id;
};

class JNIEXPORT JavaArrayListAdder final : public JavaContainerAdder
{
public:
    JavaArrayListAdder(JNIEnv* const env) noexcept;
};

class JNIEXPORT JavaListIterator final
{
public:
    JavaListIterator(JNIEnv* const env, const JniReference<jobject>& array_list) noexcept;
    JniReference<jobject> get(const JniReference<jobject>& array_list, jint index) const noexcept;
    jint length() const noexcept
    {
        return m_length;
    }

private:
    JNIEnv* const m_env;
    const JniReference<jclass> m_list_class;
    const jmethodID m_get_method_id;
    const jint m_length;
};

class JNIEXPORT JavaHashMapAdder final
{
public:
    JavaHashMapAdder(JNIEnv* const env) noexcept;
    void add(const JniReference<jobject>& key, const JniReference<jobject>& value) noexcept;
    JniReference<jobject> fetch_hash_map() noexcept
    {
        return std::move(m_java_container.instance);
    }
private:
    JavaContainer m_java_container;
    const jmethodID m_put_method_id;
};

class JNIEXPORT JavaSetIterator final
{
public:
    JavaSetIterator(JNIEnv* const env, const JniReference<jobject>& java_set) noexcept;

    bool has_next() const noexcept;

    JniReference<jobject> next() const noexcept;

private:
    JNIEnv* const m_env;
    const JniReference<jclass> m_iterator_class;

    const JniReference<jobject> m_iterator;
    const jmethodID m_has_next_method_id;
    const jmethodID m_next_method_id;
};

class JNIEXPORT JavaMapIterator final
{
public:
    JavaMapIterator(JNIEnv* const env, const JniReference<jobject>& java_map) noexcept;

    bool has_next() const noexcept
    {
        return m_set_iterator.has_next();
    }

    std::pair<JniReference<jobject>, JniReference<jobject>> next() const noexcept;
private:
    JNIEnv* const m_env;
    const JniReference<jclass> m_map_class;
    const JniReference<jclass> m_map_entry_class;
    const JavaSetIterator m_set_iterator;

    const jmethodID m_get_key_method_id;
    const jmethodID m_get_value_method_id;
};

class JNIEXPORT JavaHashSetAdder final : public JavaContainerAdder
{
public:
    JavaHashSetAdder(JNIEnv* const env) noexcept;
};

class JavaEnumSetAdder final : public JavaContainerAdder
{
public:
    JavaEnumSetAdder(JNIEnv* const env, const JniReference<jclass>& element_class) noexcept;

private:
    JavaEnumSetAdder(JNIEnv* const env,
                     std::pair<JniReference<jclass>, JniReference<jobject>> class_and_object) noexcept;
};

}
}

