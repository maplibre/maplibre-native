#pragma once

#include <mbgl/gfx/drawable.hpp>

namespace mbgl {
namespace gfx {

struct Drawable::DrawSegment {
    DrawSegment(gfx::DrawMode mode_, Segment<void>&& segment_)
        : mode(mode_),
          segment(std::move(segment_)) {}

    const gfx::DrawMode& getMode() const { return mode; }
    const Segment<void>& getSegment() const { return segment; }

protected:
    gfx::DrawMode mode;
    Segment<void> segment;
};

} // namespace gfx
} // namespace mbgl
