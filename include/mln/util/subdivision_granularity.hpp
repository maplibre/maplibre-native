#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>

namespace mln {

/// How finely tile geometry is split so the vertex shader can bend it onto the globe; port of GL JS
/// `subdivision_granularity_settings.ts`.
class SubdivisionGranularityExpression {
public:
    constexpr SubdivisionGranularityExpression(uint32_t baseZoomGranularity_, uint32_t minGranularity_)
        : baseZoomGranularity(baseZoomGranularity_),
          minGranularity(minGranularity_) {
        assert(minGranularity <= baseZoomGranularity || baseZoomGranularity == 0);
    }

    /// The base granularity halves with every zoom level, down to the minimum and never below 1.
    uint32_t getGranularityForZoomLevel(uint8_t zoom) const {
        return std::max({zoom < 32 ? baseZoomGranularity >> zoom : 0u, minGranularity, 1u});
    }

    bool operator==(const SubdivisionGranularityExpression&) const = default;

private:
    uint32_t baseZoomGranularity;
    uint32_t minGranularity;
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
        return {.fill = {128, 2}, .line = {512, 0}, .tile = {128, 32}, .stencil = {128, 1}, .circle = 3};
    }
};

} // namespace mln
