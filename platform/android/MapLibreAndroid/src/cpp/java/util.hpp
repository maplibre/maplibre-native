#pragma once

#include <mbgl/util/noncopyable.hpp>

#include <jni/jni.hpp>

namespace mln::android::java::util {

using UntypedArray_t = jni::Array<jni::Object<>>;

struct List : private mln::util::noncopyable {
    static constexpr auto Name() { return "java/util/List"; };

    template <class T, typename TypedArray_t = jni::Array<jni::Object<T>>>
    static auto toArray(jni::JNIEnv& env, const jni::Object<List>& list) -> jni::Local<TypedArray_t> {
        static const auto toArray = jni::Class<List>::Singleton(env).GetMethod<UntypedArray_t()>(env, "toArray");
        return jni::Local<TypedArray_t>(env, list.Call(env, toArray).release());
    };
};

struct Arrays : private mln::util::noncopyable {
    static constexpr auto Name() { return "java/util/Arrays"; };

    template <class T>
    static jni::Local<jni::Object<List>> asList(jni::JNIEnv& env, const jni::Array<jni::Object<T>>& array) {
        const auto& javaClass = jni::Class<Arrays>::Singleton(env);
        static auto asList = javaClass.GetStaticMethod<jni::Object<List>(UntypedArray_t)>(env, "asList");
        const auto typeErasedArray = jni::Local<UntypedArray_t>(env, jni::NewLocal(env, array).release());
        return javaClass.Call(env, asList, typeErasedArray);
    }
};

struct Set : private mln::util::noncopyable {
    static constexpr auto Name() { return "java/util/Set"; };

    template <class T, typename TypedArray_t = jni::Array<jni::Object<T>>>
    static jni::Local<TypedArray_t> toArray(jni::JNIEnv& env, const jni::Object<Set>& list) {
        static auto toArray = jni::Class<Set>::Singleton(env).GetMethod<UntypedArray_t()>(env, "toArray");
        return jni::Local<TypedArray_t>(env, list.Call(env, toArray).release());
    };
};

struct Map : private mln::util::noncopyable {
    static constexpr auto Name() { return "java/util/Map"; };

    struct Entry : private mln::util::noncopyable {
        static constexpr auto Name() { return "java/util/Map$Entry"; };

        template <class T>
        static jni::Local<jni::Object<T>> getKey(jni::JNIEnv& env, const jni::Object<Entry>& entry) {
            static auto method = jni::Class<Map::Entry>::Singleton(env).GetMethod<jni::Object<>()>(env, "getKey");
            return jni::Cast(env, jni::Class<T>::Singleton(env), entry.Call(env, method));
        }

        template <class T>
        static jni::Local<jni::Object<T>> getValue(jni::JNIEnv& env, const jni::Object<Entry>& entry) {
            static auto method = jni::Class<Map::Entry>::Singleton(env).GetMethod<jni::Object<>()>(env, "getValue");
            return jni::Cast(env, jni::Class<T>::Singleton(env), entry.Call(env, method));
        }
    };
};

struct ArrayList : private mln::util::noncopyable {
    static constexpr auto Name() { return "java/util/ArrayList"; };

    static auto New(jni::JNIEnv& env) {
        const auto& javaClass = jni::Class<ArrayList>::Singleton(env);
        static const auto constructor = javaClass.GetConstructor(env);
        return javaClass.New(env, constructor);
    }

    template <typename... Args>
    static auto New(jni::JNIEnv& env, jint initialCapacity, Args&&... args) {
        auto& javaClass = jni::Class<ArrayList>::Singleton(env);
        static const auto constructor = javaClass.GetConstructor<jint>(env);
        return javaClass.New(env, constructor, initialCapacity, std::forward<Args>(args)...);
    }

    template <typename T>
    static bool add(jni::JNIEnv& env, const jni::Object<ArrayList>& list, const jni::Object<T>& item) {
        static const auto method = jni::Class<ArrayList>::Singleton(env).GetMethod<jni::jboolean(jni::Object<>)>(env,
                                                                                                                 "add");
        return list.Call(env, method, item);
    }
};

struct HashSet : private mln::util::noncopyable {
    static constexpr auto Name() { return "java/util/HashSet"; };

    static auto New(jni::JNIEnv& env) {
        const auto& javaClass = jni::Class<HashSet>::Singleton(env);
        static const auto constructor = javaClass.GetConstructor(env);
        return javaClass.New(env, constructor);
    }

    template <typename... Args>
    static auto New(jni::JNIEnv& env, jint initialCapacity, Args&&... args) {
        const auto& javaClass = jni::Class<HashSet>::Singleton(env);
        static const auto constructor = javaClass.GetConstructor<jint>(env);
        return javaClass.New(env, constructor, initialCapacity, std::forward<Args>(args)...);
    }

    template <typename T>
    static bool add(jni::JNIEnv& env, const jni::Object<HashSet>& set, const jni::Object<T>& item) {
        static const auto method = jni::Class<HashSet>::Singleton(env).GetMethod<jni::jboolean(jni::Object<>)>(env,
                                                                                                               "add");
        return set.Call(env, method, item);
    }
};

struct HashMap : private mln::util::noncopyable {
    static constexpr auto Name() { return "java/util/HashMap"; };

    static auto New(jni::JNIEnv& env) {
        const auto& javaClass = jni::Class<HashMap>::Singleton(env);
        static const auto constructor = javaClass.GetConstructor(env);
        return javaClass.New(env, constructor);
    }

    static auto New(jni::JNIEnv& env, jint initialCapacity) {
        const auto& javaClass = jni::Class<HashMap>::Singleton(env);
        static const auto constructor = javaClass.GetConstructor<jint>(env);
        return javaClass.New(env, constructor, initialCapacity);
    }

    template <typename K, typename V>
    static std::optional<V> get(jni::JNIEnv& env, const jni::Object<HashMap>& map, const jni::Object<K>& key) {
        static auto method = jni::Class<HashMap>::Singleton(env).GetMethod<jni::Object<>(jni::Object<>)>(env, "get");
        const auto result = map.Call(env, method, key);
        return result ? static_cast<V>(*result) : std::nullopt;
    }

    template <typename K, typename V>
    static void put(jni::JNIEnv& env,
                    const jni::Object<HashMap>& map,
                    const jni::Object<K>& key,
                    const jni::Object<V>& value) {
        static const auto put =
            jni::Class<HashMap>::Singleton(env).GetMethod<jni::Object<>(jni::Object<>, jni::Object<>)>(env, "put");
        map.Call(env, put, key, value);
    }
};

void registerNative(jni::JNIEnv&);

} // namespace mln::android::java::util
