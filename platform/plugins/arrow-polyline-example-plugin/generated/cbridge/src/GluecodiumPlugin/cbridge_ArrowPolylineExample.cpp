//

//

#include "cbridge/include/GluecodiumPlugin/cbridge_ArrowPolylineExample.h"
#include "GluecodiumPlugin/ArrowPolylineConfig.h"
#include "GluecodiumPlugin/ArrowPolylineExample.h"
#include "GluecodiumPlugin/LatLng.h"
#include "cbridge_internal/include/BaseHandleImpl.h"
#include "cbridge_internal/include/TypeInitRepository.h"
#include "cbridge_internal/include/WrapperCache.h"
#include "glue_internal/TypeRepository.h"
#include "glue_internal/VectorHash.h"
#include <memory>
#include <new>
#include <vector>

void GluecodiumPlugin_ArrowPolylineExample_release_handle(_baseRef handle) {
    delete get_pointer<::std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>>(handle);
}

_baseRef GluecodiumPlugin_ArrowPolylineExample_copy_handle(_baseRef handle) {
    return handle ? reinterpret_cast<_baseRef>(checked_pointer_copy(
                        *get_pointer<::std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>>(handle)))
                  : 0;
}

const void* GluecodiumPlugin_ArrowPolylineExample_get_swift_object_from_wrapper_cache(_baseRef handle) {
    return handle ? ::glue_internal::get_wrapper_cache().get_cached_wrapper(
                        get_pointer<::std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>>(handle)->get())
                  : nullptr;
}

void GluecodiumPlugin_ArrowPolylineExample_cache_swift_object_wrapper(_baseRef handle, const void* swift_pointer) {
    if (!handle) return;
    ::glue_internal::get_wrapper_cache().cache_wrapper(
        get_pointer<::std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>>(handle)->get(), swift_pointer);
}

void GluecodiumPlugin_ArrowPolylineExample_remove_swift_object_from_wrapper_cache(_baseRef handle) {
    if (!::glue_internal::WrapperCache::is_alive) return;
    ::glue_internal::get_wrapper_cache().remove_cached_wrapper(
        get_pointer<::std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>>(handle)->get());
}

extern "C" {
extern void* _CBridgeInitGluecodiumPlugin_ArrowPolylineExample(_baseRef handle);
}

namespace {
struct GluecodiumPlugin_ArrowPolylineExampleRegisterInit {
    GluecodiumPlugin_ArrowPolylineExampleRegisterInit() {
        get_init_repository().add_init("GluecodiumPlugin_ArrowPolylineExample",
                                       &_CBridgeInitGluecodiumPlugin_ArrowPolylineExample);
    }
} s_GluecodiumPlugin_ArrowPolylineExample_register_init;
} // namespace

void* GluecodiumPlugin_ArrowPolylineExample_get_typed(_baseRef handle) {
    const auto& real_type_id = ::glue_internal::get_type_repository().get_id(
        get_pointer<::std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>>(handle)->get());
    auto init_function = get_init_repository().get_init(real_type_id);
    return init_function ? init_function(handle) : _CBridgeInitGluecodiumPlugin_ArrowPolylineExample(handle);
}

_baseRef GluecodiumPlugin_ArrowPolylineExample_create() {
    return Conversion<::std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>>::toBaseRef(
        ::GluecodiumPlugin::ArrowPolylineExample::create());
}

void GluecodiumPlugin_ArrowPolylineExample_addArrowPolyline(_baseRef _instance, _baseRef coordinates, _baseRef config) {
    return get_pointer<::std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>>(_instance)
        ->get()
        ->add_arrow_polyline(Conversion<::std::vector<::GluecodiumPlugin::LatLng>>::toCpp(coordinates),
                             Conversion<::GluecodiumPlugin::ArrowPolylineConfig>::toCpp(config));
}

void GluecodiumPlugin_ArrowPolylineExample_removeArrowPolyline(_baseRef _instance) {
    return get_pointer<::std::shared_ptr<::GluecodiumPlugin::ArrowPolylineExample>>(_instance)
        ->get()
        ->remove_arrow_polyline();
}
