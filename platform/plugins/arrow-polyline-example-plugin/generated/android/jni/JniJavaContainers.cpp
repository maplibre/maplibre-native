/*

 *
 */

#include "JniJavaContainers.h"

#include "JniCallJavaMethod.h"

namespace glue_internal
{
namespace jni
{

JavaContainer::JavaContainer(JNIEnv* const env, const char* class_name) noexcept
    : env(env)
    , container_class(find_class(env, class_name))
    , instance(create_object(env, container_class))
{
}

JavaContainer::JavaContainer(JNIEnv* const env, JniReference<jclass> container_class, JniReference<jobject> instance) noexcept
    : env(env)
    , container_class(std::move(container_class))
    , instance(std::move(instance))
{
}

namespace
{
jmethodID get_add_method_id(JNIEnv* const env, const JniReference<jclass>& container_class)
{
    return env->GetMethodID(container_class.get(), "add", "(Ljava/lang/Object;)Z");
}
} // namespace

JavaContainerAdder::JavaContainerAdder(JNIEnv* const env, const char* class_name) noexcept
    : m_java_container{env, class_name}
    , m_add_method_id(get_add_method_id(env, m_java_container.container_class))
{
}

JavaContainerAdder::JavaContainerAdder(JNIEnv* const env,
                                       JniReference<jclass> container_class,
                                       JniReference<jobject> instance) noexcept
    : m_java_container{env, std::move(container_class), std::move(instance)}
    , m_add_method_id(get_add_method_id(env, m_java_container.container_class))
{
}

void JavaContainerAdder::add(const JniReference<jobject>& item) noexcept
{
    call_java_method<jboolean>(m_java_container.env, m_java_container.instance, m_add_method_id, item);
}

JavaArrayListAdder::JavaArrayListAdder(JNIEnv* const env) noexcept
    : JavaContainerAdder(env, "java/util/ArrayList")
{
}

JavaListIterator::JavaListIterator(JNIEnv* const env, const JniReference<jobject>& array_list) noexcept
    : m_env(env)
    , m_list_class(find_class(env, "java/util/List"))
    , m_get_method_id(m_env->GetMethodID(m_list_class.get(), "get", "(I)Ljava/lang/Object;"))
    , m_length(call_java_method<jint>(m_env, m_list_class, array_list, "size", "()I"))
{
}

JniReference<jobject> JavaListIterator::get(const JniReference<jobject>& array_list, jint index) const noexcept
{
    return call_java_method<jobject>(m_env, array_list, m_get_method_id, index);
}

JavaHashMapAdder::JavaHashMapAdder(JNIEnv* const env) noexcept
    : m_java_container(env, "java/util/HashMap")
    , m_put_method_id(env->GetMethodID(m_java_container.container_class.get(), "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;"))
{
}

void JavaHashMapAdder::add(const JniReference<jobject>& key, const JniReference<jobject>& value) noexcept
{
    call_java_method<jobject>(m_java_container.env, m_java_container.instance, m_put_method_id, key, value);
}

JavaSetIterator::JavaSetIterator(JNIEnv* const env, const JniReference<jobject>& java_set) noexcept
    : m_env(env)
    , m_iterator_class(find_class(m_env, "java/util/Iterator"))
    , m_iterator(call_java_method<jobject>(m_env, "java/util/Set", java_set, "iterator", "()Ljava/util/Iterator;"))
    , m_has_next_method_id(m_env->GetMethodID(m_iterator_class.get(), "hasNext", "()Z"))
    , m_next_method_id(m_env->GetMethodID(m_iterator_class.get(), "next", "()Ljava/lang/Object;"))
{
}

bool JavaSetIterator::has_next() const noexcept
{
    return call_java_method<jboolean>( m_env, m_iterator, m_has_next_method_id );
}

JniReference<jobject> JavaSetIterator::next() const noexcept
{
    return call_java_method<jobject>( m_env, m_iterator, m_next_method_id );
}

namespace
{
JniReference<jobject> get_entry_set(JNIEnv* const env,
                                    const JniReference<jclass>& map_class,
                                    const JniReference<jobject>& java_map) noexcept
{
    return call_java_method<jobject>(env, map_class, java_map, "entrySet", "()Ljava/util/Set;" );
}
} // namespace

JavaMapIterator::JavaMapIterator(JNIEnv* const env, const JniReference<jobject>& java_map) noexcept
    : m_env(env)
    , m_map_class(find_class(m_env, "java/util/Map"))
    , m_map_entry_class(find_class(m_env, "java/util/Map$Entry"))
    , m_set_iterator(env, get_entry_set(m_env, m_map_class, java_map))
    , m_get_key_method_id(m_env->GetMethodID(m_map_entry_class.get(), "getKey", "()Ljava/lang/Object;"))
    , m_get_value_method_id(m_env->GetMethodID(m_map_entry_class.get(), "getValue", "()Ljava/lang/Object;"))
{
}

std::pair<JniReference<jobject>, JniReference<jobject>> JavaMapIterator::next() const noexcept
{
    const auto& entry = m_set_iterator.next();
    return {call_java_method<jobject>( m_env, entry, m_get_key_method_id ),
            call_java_method<jobject>( m_env, entry, m_get_value_method_id )};
}

JavaHashSetAdder::JavaHashSetAdder(JNIEnv* const env) noexcept
    : JavaContainerAdder(env, "java/util/HashSet")
{
}

namespace
{
std::pair<JniReference<jclass>, JniReference<jobject>>
make_enum_set(JNIEnv* const env,
              const JniReference<jclass>& element_class)
{
    auto enum_set_class = find_class(env, "java/util/EnumSet");
    const jmethodID none_of_method_id = env->GetStaticMethodID(
        enum_set_class.get(), "noneOf", "(Ljava/lang/Class;)Ljava/util/EnumSet;");
    auto enum_set = make_local_ref(env, env->CallStaticObjectMethod(
        enum_set_class.get(), none_of_method_id, element_class.get()));
    
    return {std::move(enum_set_class), std::move(enum_set)};
}
} // namespace

JavaEnumSetAdder::JavaEnumSetAdder(JNIEnv* const env,
                                   const JniReference<jclass>& element_class) noexcept
    : JavaEnumSetAdder(env, make_enum_set(env, element_class))
{
}

JavaEnumSetAdder::JavaEnumSetAdder(JNIEnv* const env,
                                   std::pair<JniReference<jclass>, JniReference<jobject>> class_and_object) noexcept
    : JavaContainerAdder(env, std::move(class_and_object.first), std::move(class_and_object.second))
{
}

}
}
