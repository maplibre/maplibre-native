#pragma once

#include <mln/tile/geometry_tile_data.hpp>

namespace mln {

class Anchor;

bool checkMaxAngle(
    const GeometryCoordinates& line, const Anchor& anchor, float labelLength, float windowSize, float maxAngle);

} // namespace mln
