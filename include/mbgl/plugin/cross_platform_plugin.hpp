#pragma once

#include <mbgl/map/map.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/renderer/renderer.hpp>

namespace mbgl::platform {

class XPlatformPlugin : public mbgl::MapObserver {
public:
    virtual ~XPlatformPlugin() = default;
    virtual void onLoad(mbgl::Map* map, mbgl::Renderer* renderer) = 0;
    virtual void onUnload() = 0;
};

} // namespace mbgl::platform
