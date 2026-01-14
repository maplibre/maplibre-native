#ifndef plugin_platform_darwin_hpp
#define plugin_platform_darwin_hpp

#include <Metal/Metal.h>
#include <stdio.h>
#include <mbgl/plugin/plugin_map_layer.hpp>

namespace mbgl {

namespace plugin {

class RenderingContextMetal: public RenderingContext {
public:
    id<MTLRenderCommandEncoder> renderEncoder;
    id<MTLDevice> metalDevice;
};
    

}

}

#endif /* plugin_platform_darwin_hpp */
