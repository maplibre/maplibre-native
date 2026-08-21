#pragma once

#include <mln/gfx/drawable_data.hpp>
#include <mln/geometry/line_atlas.hpp>

#include <memory>

namespace mln {

namespace gfx {

class LineDrawableData : public DrawableData {
public:
    LineDrawableData(LinePatternCap linePatternCap_)
        : linePatternCap(linePatternCap_) {}

    LinePatternCap linePatternCap;
};

using UniqueLineDrawableData = std::unique_ptr<LineDrawableData>;

} // namespace gfx
} // namespace mln
