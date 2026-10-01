#pragma once

#include <mln/gfx/draw_mode.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/shaders/segment.hpp>

namespace mln {
namespace gfx {

struct Drawable::DrawSegment {
    DrawSegment(gfx::DrawMode mode_, SegmentBase&& segment_)
        : mode(mode_),
          segment(std::move(segment_)) {}

    virtual ~DrawSegment() = default;

    const gfx::DrawMode& getMode() const { return mode; }

    SegmentBase& getSegment() { return segment; }
    const SegmentBase& getSegment() const { return segment; }

protected:
    gfx::DrawMode mode;
    SegmentBase segment;
};

} // namespace gfx
} // namespace mln
