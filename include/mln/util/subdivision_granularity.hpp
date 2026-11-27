#pragma once

#include <mln/util/constants.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>

namespace mln {

/// How finely tile geometry is split so the vertex shader can bend it onto the globe; port of GL JS
/// `subdivision_granularity_settings.ts`.
class SubdivisionGranularityExpression {
public:
    constexpr SubdivisionGranularityExpression(uint32_t baseZoomGranularity_,
                                               uint32_t minGranularity_,
                                               uint32_t surfaceGranularity_ = 0)
        : baseZoomGranularity(baseZoomGranularity_),
          minGranularity(minGranularity_),
          surfaceGranularity(surfaceGranularity_) {
        assert(minGranularity <= baseZoomGranularity || baseZoomGranularity == 0);
    }

    /// The base granularity halves with every zoom level, down to the minimum and never below 1. The surface
    /// granularity halves every second zoom level: a chord's distance from the curved surface falls with the square of
    /// its length, so this keeps that distance about the same in pixels at every zoom level. A tile gets the finer of
    /// the two. A build without the globe never subdivides, which lets the linker drop the subdivision code.
    uint32_t getGranularityForZoomLevel(uint8_t zoom) const {
        return util::GLOBE_ENABLED ? std::max({zoom < 32 ? baseZoomGranularity >> zoom : 0u,
                                               zoom / 2 < 32 ? surfaceGranularity >> (zoom / 2) : 0u,
                                               minGranularity,
                                               1u})
                                   : 1u;
    }

    bool operator==(const SubdivisionGranularityExpression&) const = default;

private:
    uint32_t baseZoomGranularity;
    uint32_t minGranularity;
    uint32_t surfaceGranularity;
};

struct SubdivisionGranularitySetting {
    SubdivisionGranularityExpression fill{0, 0};
    SubdivisionGranularityExpression line{0, 0};
    SubdivisionGranularityExpression tile{0, 0};
    SubdivisionGranularityExpression stencil{0, 0};
    /// Circle vertices per axis: 1, 3, 5 or 7.
    uint8_t circle = 1;

    bool operator==(const SubdivisionGranularitySetting&) const = default;

    static constexpr SubdivisionGranularitySetting none() { return {}; }

    /// GL JS's globe values.
    static constexpr SubdivisionGranularitySetting globe() {
        return {.fill = {128, 2, 32}, .line = {512, 0}, .tile = {128, 32}, .stencil = {128, 1, 32}, .circle = 3};
    }
};

} // namespace mln
