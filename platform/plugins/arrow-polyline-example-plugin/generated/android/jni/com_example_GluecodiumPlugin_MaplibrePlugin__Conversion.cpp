/*

 *
 */

#include "com_example_GluecodiumPlugin_MaplibrePlugin__Conversion.h"
#include "CppProxyBase.h"
#include "FieldAccessMethods.h"
#include "JniClassCache.h"
#include "JniNativeHandle.h"
#include "JniThrowNewException.h"
#include "JniWrapperCache.h"
#include <new>

namespace glue_internal {
namespace jni {

REGISTER_JNI_CLASS_CACHE_INHERITANCE("com/example/GluecodiumPlugin/MaplibrePlugin",
                                     com_example_GluecodiumPlugin_MaplibrePlugin,
                                     "GluecodiumPlugin_MaplibrePlugin",
                                     ::GluecodiumPlugin::MaplibrePlugin)

std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin> convert_from_jni(
    JNIEnv* _env, const JniReference<jobject>& _jobj, TypeId<std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>>) {
    std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin> _nresult{};
    auto& nativeBaseClass = get_cached_native_base_class();
    if (_env->IsInstanceOf(_jobj.get(), nativeBaseClass.get())) {
        if (_jobj != nullptr) {
            auto long_ptr = get_class_native_handle(_env, _jobj);
            _nresult = *reinterpret_cast<std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>*>(long_ptr);
        }
    } else {
        createCppProxy(_env, _jobj, _nresult);
    }
    return _nresult;
}

JniReference<jobject> convert_to_jni(JNIEnv* _jenv,
                                     const std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>& _ninput) {
    if (!_ninput) {
        return {};
    }

    auto jResult = ::glue_internal::jni::CppProxyBase::getJavaObject(_jenv, _ninput.get());
    if (jResult) return jResult;

    jResult = ::glue_internal::jni::JniWrapperCache::get_cached_wrapper(_jenv, _ninput);
    if (jResult) return jResult;

    const auto& id = ::glue_internal::get_type_repository().get_id(_ninput.get());
    const auto& javaClass = CachedJavaClass<::GluecodiumPlugin::MaplibrePlugin>::get_java_class(id);
    auto pInstanceSharedPointer = new (::std::nothrow) std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>(_ninput);
    if (pInstanceSharedPointer == nullptr) {
        throw_new_out_of_memory_exception(_jenv);
    }
    jResult = ::glue_internal::jni::create_instance_object(
        _jenv, javaClass, reinterpret_cast<jlong>(pInstanceSharedPointer));
    ::glue_internal::jni::JniWrapperCache::cache_wrapper(_jenv, _ninput, jResult);

    return jResult;
}

} // namespace jni
} // namespace glue_internal
