#pragma once

#include <mln/geometry/anchor.hpp>
#include <mln/tile/geometry_tile_data.hpp>
#include <mln/util/math.hpp>

namespace mln {

Anchors getAnchors(const GeometryCoordinates& line,
                   float spacing,
                   float maxAngle,
                   float textLeft,
                   float textRight,
                   float iconLeft,
                   float iconRight,
                   float glyphSize,
                   float boxScale,
                   float overscaling);

std::optional<Anchor> getCenterAnchor(const GeometryCoordinates& line,
                                      float maxAngle,
                                      float textLeft,
                                      float textRight,
                                      float iconLeft,
                                      float iconRight,
                                      float glyphSize,
                                      float boxScale);

} // namespace mln
