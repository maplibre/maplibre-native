#pragma once

#include <mln/gfx/drawable_data.hpp>
#include <mln/plugin/plugin_api.h>

#include <cstdint>
#include <string>

namespace mln {
namespace gfx {

/**
 * Immutable, host-owned inputs associated with one drawable generated from a
 * plugin render graph. Backend resources intentionally do not cross the
 * plugin boundary; the plugin only sees these values through its uniform
 * callback while the drawable is being tweaked.
 */
class PluginRenderGraphDrawableData final : public DrawableData {
public:
    explicit PluginRenderGraphDrawableData(std::string shaderID_)
        : shaderID(std::move(shaderID_)) {}

    PluginRenderGraphDrawableData(std::string shaderID_,
                                  uint32_t passID_,
                                  uint32_t dimension_,
                                  uint32_t stride_,
                                  mln_plugin_raster_dem_encoding encoding_,
                                  uint32_t sourceMaxZoom_,
                                  uint32_t renderTargetWidth_ = 0,
                                  uint32_t renderTargetHeight_ = 0)
        : shaderID(std::move(shaderID_)),
          passID(passID_),
          dimension(dimension_),
          stride(stride_),
          encoding(encoding_),
          sourceMaxZoom(sourceMaxZoom_),
          renderTargetWidth(renderTargetWidth_),
          renderTargetHeight(renderTargetHeight_) {}

    std::string shaderID;
    uint32_t passID = 0;
    uint32_t dimension = 0;
    uint32_t stride = 0;
    mln_plugin_raster_dem_encoding encoding = MLN_PLUGIN_RASTER_DEM_MAPBOX;
    uint32_t sourceMaxZoom = 0;
    uint32_t renderTargetWidth = 0;
    uint32_t renderTargetHeight = 0;
};

} // namespace gfx
} // namespace mln
