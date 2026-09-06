#pragma once

#include <mln/gfx/drawable_data.hpp>
#include <mln/plugin/plugin_api.h>
#include <mln/util/mat4.hpp>

#include <algorithm>
#include <cstdint>

namespace mln {
namespace gfx {

/** Internal metadata attached by geometry adapters to ordinary drawables. */
class PluginDrawableData final : public DrawableData {
public:
    PluginDrawableData(mln_plugin_draw_packet_kind kind_,
                       bool instanced_,
                       int8_t wallVertexLocation_,
                       int8_t positionLocation_,
                       int8_t decimalsLocation_,
                       int8_t normalLocation_,
                       int8_t baseLocation_,
                       int8_t heightLocation_)
        : instanced(instanced_),
          wallVertexLocation(wallVertexLocation_),
          positionLocation(positionLocation_),
          decimalsLocation(decimalsLocation_),
          normalLocation(normalLocation_),
          baseLocation(baseLocation_),
          heightLocation(heightLocation_) {
        packet.struct_size = sizeof(packet);
        packet.kind = kind_;
    }

    PluginDrawableData* getPluginData() override { return this; }
    const PluginDrawableData* getPluginData() const override { return this; }

    void setMatrix(const mat4& matrix) {
        std::transform(
            matrix.begin(), matrix.end(), packet.tile_matrix, [](double value) { return static_cast<float>(value); });
    }

    mln_plugin_draw_packet_v1 packet{};
    bool instanced = false;
    int8_t wallVertexLocation = -1;
    int8_t positionLocation = -1;
    int8_t decimalsLocation = -1;
    int8_t normalLocation = -1;
    int8_t baseLocation = -1;
    int8_t heightLocation = -1;
};

} // namespace gfx
} // namespace mln
