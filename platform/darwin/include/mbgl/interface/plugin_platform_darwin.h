#pragma once

#if defined(__cplusplus) && defined(__OBJC__)

#include <Metal/Metal.h>

#if __has_include(<MapLibre/plugin_map_layer.h>)
#include <MapLibre/plugin_map_layer.h>
#else
#include <mbgl/plugin/plugin_map_layer.h>
#endif

namespace mbgl {
namespace plugin {

class RenderingContextMetal : public RenderingContext {
public:
    id<MTLRenderCommandEncoder> renderEncoder;
    id<MTLDevice> metalDevice;
};

} // namespace plugin
} // namespace mbgl

#endif
