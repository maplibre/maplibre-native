#pragma once

#include <mbgl/util/geometry.hpp>

#include <utility>
#include <vector>
#include <optional>

namespace mbgl {

class Anchor {
public:
    Point<float> point;
    float angle = 0.0f;
    std::optional<std::size_t> segment;

    Anchor(float x_, float y_, float angle_, std::optional<std::size_t> segment_ = std::nullopt)
        : point(x_, y_),
          angle(angle_),
          segment(std::move(segment_)) {}
};

using Anchors = std::vector<Anchor>;

} // namespace mbgl
