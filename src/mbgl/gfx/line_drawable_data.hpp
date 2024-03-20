#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/geometry/line_atlas.hpp>

#include <memory>

namespace mbgl {

namespace gfx {

class LineDrawableData : public DrawableData {
public:
    LineDrawableData(LinePatternCap linePatternCap_)
        : linePatternCap(linePatternCap_) {}

    LinePatternCap linePatternCap;
};

using UniqueLineDrawableData = std::unique_ptr<LineDrawableData>;

} // namespace gfx
} // namespace mbgl
