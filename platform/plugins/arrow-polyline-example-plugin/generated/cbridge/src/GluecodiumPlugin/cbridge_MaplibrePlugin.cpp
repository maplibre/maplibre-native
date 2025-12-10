//

//

#include "cbridge/include/GluecodiumPlugin/cbridge_MaplibrePlugin.h"
#include "GluecodiumPlugin/MaplibrePlugin.h"
#include "cbridge_internal/include/BaseHandleImpl.h"
#include "cbridge_internal/include/TypeInitRepository.h"
#include "cbridge_internal/include/WrapperCache.h"
#include "glue_internal/TypeRepository.h"
#include <cstdint>
#include <memory>
#include <new>

void GluecodiumPlugin_MaplibrePlugin_release_handle(_baseRef handle) {
    delete get_pointer<::std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>>(handle);
}

_baseRef GluecodiumPlugin_MaplibrePlugin_copy_handle(_baseRef handle) {
    return handle ? reinterpret_cast<_baseRef>(checked_pointer_copy(
                        *get_pointer<::std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>>(handle)))
                  : 0;
}

const void* GluecodiumPlugin_MaplibrePlugin_get_swift_object_from_wrapper_cache(_baseRef handle) {
    return handle ? ::glue_internal::get_wrapper_cache().get_cached_wrapper(
                        get_pointer<::std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>>(handle)->get())
                  : nullptr;
}

void GluecodiumPlugin_MaplibrePlugin_cache_swift_object_wrapper(_baseRef handle, const void* swift_pointer) {
    if (!handle) return;
    ::glue_internal::get_wrapper_cache().cache_wrapper(
        get_pointer<::std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>>(handle)->get(), swift_pointer);
}

void GluecodiumPlugin_MaplibrePlugin_remove_swift_object_from_wrapper_cache(_baseRef handle) {
    if (!::glue_internal::WrapperCache::is_alive) return;
    ::glue_internal::get_wrapper_cache().remove_cached_wrapper(
        get_pointer<::std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>>(handle)->get());
}

extern "C" {
extern void* _CBridgeInitGluecodiumPlugin_MaplibrePlugin(_baseRef handle);
}

namespace {
struct GluecodiumPlugin_MaplibrePluginRegisterInit {
    GluecodiumPlugin_MaplibrePluginRegisterInit() {
        get_init_repository().add_init("GluecodiumPlugin_MaplibrePlugin", &_CBridgeInitGluecodiumPlugin_MaplibrePlugin);
    }
} s_GluecodiumPlugin_MaplibrePlugin_register_init;
} // namespace

void* GluecodiumPlugin_MaplibrePlugin_get_typed(_baseRef handle) {
    const auto& real_type_id = ::glue_internal::get_type_repository().get_id(
        get_pointer<::std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>>(handle)->get());
    auto init_function = get_init_repository().get_init(real_type_id);
    return init_function ? init_function(handle) : _CBridgeInitGluecodiumPlugin_MaplibrePlugin(handle);
}

_baseRef GluecodiumPlugin_MaplibrePlugin_create() {
    return Conversion<::std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>>::toBaseRef(
        ::GluecodiumPlugin::MaplibrePlugin::create());
}

uint64_t GluecodiumPlugin_MaplibrePlugin_ptr_get(_baseRef _instance) {
    return get_pointer<::std::shared_ptr<::GluecodiumPlugin::MaplibrePlugin>>(_instance)->get()->get_ptr();
}
