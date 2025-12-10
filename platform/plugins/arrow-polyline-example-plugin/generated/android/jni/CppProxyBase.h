/*

 *
 */

#pragma once

#include <jni.h>

#include "JniTemplateMetainfo.h"
#include "JniReference.h"
#include "JniCallJavaMethod.h"
#include "JniCallbackErrorChecking.h"

#include <memory>
#include <new>
#include <string>

namespace glue_internal {
namespace jni {

class JNIEXPORT CppProxyBase {
private:
    using ReverseCacheKey = const void*;
    using ProxyFactoryResult = std::pair<std::shared_ptr<CppProxyBase>, ReverseCacheKey>;
    using ProxyFactoryFun = ProxyFactoryResult (*)(JniReference<jobject>, jint);

    template <typename ResultType, typename ImplType>
    struct ProxyFactory {
        static ProxyFactoryResult create(JniReference<jobject> globalRef, jint jHashCode) {
            ImplType* const newProxyInstance = new (::std::nothrow) ImplType{std::move(globalRef), jHashCode};
            const ResultType* const castedToResult = newProxyInstance;
            return std::make_pair(std::shared_ptr<ImplType>{newProxyInstance}, castedToResult);
        }
    };

    static std::shared_ptr<CppProxyBase> createProxyImpl(JNIEnv* jenv,
                                                         const JniReference<jobject>& jobj,
                                                         const ::std::string& type_key,
                                                         bool do_cache,
                                                         ProxyFactoryFun factory);

    template <typename ResultType, typename ImplType>
    static void createProxy(JNIEnv* jenv,
                            const JniReference<jobject>& jobj,
                            const ::std::string& type_key,
                            bool do_cache,
                            ::std::shared_ptr<ResultType>& result) {
        if (auto proxy = createProxyImpl(jenv, jobj, type_key, do_cache, &ProxyFactory<ResultType, ImplType>::create)) {
            result = ::std::static_pointer_cast<ImplType>(proxy);
            ;
        }
    }

public:
    template <typename ResultType, typename ImplType>
    static void createProxy(JNIEnv* jenv,
                            const JniReference<jobject>& jobj,
                            const ::std::string& type_key,
                            ::std::shared_ptr<ResultType>& result) {
        createProxy<ResultType, ImplType>(jenv, jobj, type_key, true, result);
    }

    template <typename ResultType, typename ImplType>
    static void createProxyNoCache(JNIEnv* jenv,
                                   const JniReference<jobject>& jobj,
                                   const ::std::string& type_key,
                                   ::std::shared_ptr<ResultType>& result) {
        createProxy<ResultType, ImplType>(jenv, jobj, type_key, false, result);
    }

    template <class T>
    static JniReference<jobject> getJavaObject(JNIEnv* jenv, T* interfacePtr) {
        return getJavaObjectFromReverseCache(jenv, interfacePtr);
    }

protected:
    CppProxyBase(JniReference<jobject> globalRef, jint jHashCode, ::std::string type_key) noexcept;

    virtual ~CppProxyBase();

    template <typename ResultType, class... Args>
    typename std::conditional<IsDerivedFromJObject<ResultType>::value, JniReference<ResultType>, ResultType>::type
    callJavaMethod(const char* methodName, const char* jniSignature, JNIEnv* jniEnv, const Args&... args) const {
        return call_java_method<ResultType>(jniEnv, mGlobalRef, methodName, jniSignature, args...);
    }

    static JNIEnv* getJniEnvironment() noexcept;

private:
    static jint getHashCode(JNIEnv* jniEnv, jobject jObj);

    static void registerInReverseCache(CppProxyBase* proxyBase, ReverseCacheKey reverseCacheKey, jobject jObj);
    void removeSelfFromReverseCache();
    static JniReference<jobject> getJavaObjectFromReverseCache(JNIEnv* jniEnv, ReverseCacheKey reverseCacheKey);

private:
    const JniReference<jobject> mGlobalRef;
    const jint jHashCode;
    ReverseCacheKey mReverseCacheKey = nullptr;
    const ::std::string type_key;
};

} // namespace jni
} // namespace glue_internal
