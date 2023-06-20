#pragma once

#include <mapbox/geometry/point.hpp>
#include <mapbox/geometry/box.hpp>
#include <mbgl/math/minmax.hpp>

#include <unordered_set>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <functional>
#include <optional>
#include <cassert>

namespace mbgl {

namespace geometry {

template <typename T>
struct circle {
    using point_type = mapbox::geometry::point<T>;

    constexpr circle(point_type const& center_, T const& radius_)
        : center(center_),
          radius(radius_) {}

    point_type center;
    T radius;
};

template <typename T>
constexpr bool operator==(circle<T> const& lhs, circle<T> const& rhs) {
    return lhs.center == rhs.center && lhs.radius == rhs.radius;
}

template <typename T>
constexpr bool operator!=(circle<T> const& lhs, circle<T> const& rhs) {
    return lhs.center != rhs.center || lhs.radius != rhs.radius;
}

} // namespace geometry

/*
 GridIndex is a data structure for testing the intersection of
 circles and rectangles in a 2d plane.
 It is optimized for rapid insertion and querying.
 GridIndex splits the plane into a set of "cells" and keeps track
 of which geometries intersect with each cell. At query time,
 full geometry comparisons are only done for items that share
 at least one cell. As long as the geometries are relatively
 uniformly distributed across the plane, this greatly reduces
 the number of comparisons necessary.
*/

template <class T>
class GridIndex {
public:
    GridIndex(float width_, float height_, uint32_t cellSize_);

    using BBox = mapbox::geometry::box<float>;
    using BCircle = geometry::circle<float>;

    void insert(T&& t, const BBox&);
    void insert(T&& t, const BCircle&);

    std::vector<T> query(const BBox&) const;
    std::vector<std::pair<T, BBox>> queryWithBoxes(const BBox&) const;

    bool hitTest(const BBox&, std::optional<std::function<bool(const T&)>> predicate = std::nullopt) const;
    bool hitTest(const BCircle&, std::optional<std::function<bool(const T&)>> predicate = std::nullopt) const;

    bool empty() const;

private:
    bool noIntersection(const BBox& queryBBox) const;
    bool completeIntersection(const BBox& queryBBox) const;
    BBox convertToBox(const BCircle& circle) const;

    void query(const BBox&, std::function<bool(const T&, const BBox&)>) const;
    void query(const BCircle&, std::function<bool(const T&, const BBox&)>) const;

    std::size_t convertToXCellCoord(float x) const;
    std::size_t convertToYCellCoord(float y) const;

    bool boxesCollide(const BBox&, const BBox&) const;
    bool circlesCollide(const BCircle&, const BCircle&) const;
    bool circleAndBoxCollide(const BCircle&, const BBox&) const;

    const float width;
    const float height;

    const std::size_t xCellCount;
    const std::size_t yCellCount;
    const double xScale;
    const double yScale;

    std::vector<std::pair<T, BBox>> boxElements;
    std::vector<std::pair<T, BCircle>> circleElements;

    std::vector<std::vector<size_t>> boxCells;
    std::vector<std::vector<size_t>> circleCells;
};

template <class T>
GridIndex<T>::GridIndex(const float width_, const float height_, const uint32_t cellSize_)
    : width(width_),
      height(height_),
      xCellCount(static_cast<size_t>(std::ceil(width / cellSize_))),
      yCellCount(static_cast<size_t>(std::ceil(height / cellSize_))),
      xScale(xCellCount / width),
      yScale(yCellCount / height) {
    assert(width > 0.0f);
    assert(height > 0.0f);
    boxCells.resize(xCellCount * yCellCount);
    circleCells.resize(xCellCount * yCellCount);
}

template <class T>
void GridIndex<T>::insert(T&& t, const BBox& bbox) {
    size_t uid = boxElements.size();

    auto cx1 = convertToXCellCoord(bbox.min.x);
    auto cy1 = convertToYCellCoord(bbox.min.y);
    auto cx2 = convertToXCellCoord(bbox.max.x);
    auto cy2 = convertToYCellCoord(bbox.max.y);

    std::size_t x;
    std::size_t y;
    std::size_t cellIndex;
    for (x = cx1; x <= cx2; ++x) {
        for (y = cy1; y <= cy2; ++y) {
            cellIndex = xCellCount * y + x;
            boxCells[cellIndex].push_back(uid);
        }
    }

    boxElements.emplace_back(t, bbox);
}

template <class T>
void GridIndex<T>::insert(T&& t, const BCircle& bcircle) {
    size_t uid = circleElements.size();

    auto cx1 = convertToXCellCoord(bcircle.center.x - bcircle.radius);
    auto cy1 = convertToYCellCoord(bcircle.center.y - bcircle.radius);
    auto cx2 = convertToXCellCoord(bcircle.center.x + bcircle.radius);
    auto cy2 = convertToYCellCoord(bcircle.center.y + bcircle.radius);

    std::size_t x;
    std::size_t y;
    std::size_t cellIndex;
    for (x = cx1; x <= cx2; ++x) {
        for (y = cy1; y <= cy2; ++y) {
            cellIndex = xCellCount * y + x;
            circleCells[cellIndex].push_back(uid);
        }
    }

    circleElements.emplace_back(t, bcircle);
}

template <class T>
std::vector<T> GridIndex<T>::query(const BBox& queryBBox) const {
    std::vector<T> result;
    query(queryBBox, [&](const T& t, const BBox&) -> bool {
        result.push_back(t);
        return false;
    });
    return result;
}

template <class T>
std::vector<std::pair<T, typename GridIndex<T>::BBox>> GridIndex<T>::queryWithBoxes(const BBox& queryBBox) const {
    std::vector<std::pair<T, BBox>> result;
    query(queryBBox, [&](const T& t, const BBox& bbox) -> bool {
        result.push_back(std::make_pair(t, bbox));
        return false;
    });
    return result;
}

template <class T>
bool GridIndex<T>::hitTest(const BBox& queryBBox, std::optional<std::function<bool(const T&)>> predicate) const {
    bool hit = false;
    query(queryBBox, [&](const T& t, const BBox&) -> bool {
        if (!predicate || (*predicate)(t)) {
            hit = true;
            return true;
        } else {
            return false;
        }
    });
    return hit;
}

template <class T>
bool GridIndex<T>::hitTest(const BCircle& queryBCircle, std::optional<std::function<bool(const T&)>> predicate) const {
    bool hit = false;
    query(queryBCircle, [&](const T& t, const BBox&) -> bool {
        if (!predicate || (*predicate)(t)) {
            hit = true;
            return true;
        } else {
            return false;
        }
    });
    return hit;
}

template <class T>
bool GridIndex<T>::noIntersection(const BBox& queryBBox) const {
    return queryBBox.max.x < 0 || queryBBox.min.x >= width || queryBBox.max.y < 0 || queryBBox.min.y >= height;
}

template <class T>
bool GridIndex<T>::completeIntersection(const BBox& queryBBox) const {
    return queryBBox.min.x <= 0 && queryBBox.min.y <= 0 && width <= queryBBox.max.x && height <= queryBBox.max.y;
}

template <class T>
typename GridIndex<T>::BBox GridIndex<T>::convertToBox(const BCircle& circle) const {
    return BBox{{circle.center.x - circle.radius, circle.center.y - circle.radius},
                {circle.center.x + circle.radius, circle.center.y + circle.radius}};
}

template <class T>
void GridIndex<T>::query(const BBox& queryBBox, std::function<bool(const T&, const BBox&)> resultFn) const {
    std::unordered_set<size_t> seenBoxes;
    std::unordered_set<size_t> seenCircles;

    if (noIntersection(queryBBox)) {
        return;
    } else if (completeIntersection(queryBBox)) {
        for (auto& element : boxElements) {
            if (resultFn(element.first, element.second)) {
                return;
            }
        }
        for (auto& element : circleElements) {
            if (resultFn(element.first, convertToBox(element.second))) {
                return;
            }
        }
        return;
    }

    auto cx1 = convertToXCellCoord(queryBBox.min.x);
    auto cy1 = convertToYCellCoord(queryBBox.min.y);
    auto cx2 = convertToXCellCoord(queryBBox.max.x);
    auto cy2 = convertToYCellCoord(queryBBox.max.y);

    std::size_t x;
    std::size_t y;
    std::size_t cellIndex;
    for (x = cx1; x <= cx2; ++x) {
        for (y = cy1; y <= cy2; ++y) {
            cellIndex = xCellCount * y + x;
            // Look up other boxes
            for (auto uid : boxCells[cellIndex]) {
                if (seenBoxes.count(uid) == 0) {
                    seenBoxes.insert(uid);

                    auto& pair = boxElements.at(uid);
                    auto& bbox = pair.second;
                    if (boxesCollide(queryBBox, bbox)) {
                        if (resultFn(pair.first, bbox)) {
                            return;
                        }
                    }
                }
            }

            // Look up circles
            for (auto uid : circleCells[cellIndex]) {
                if (seenCircles.count(uid) == 0) {
                    seenCircles.insert(uid);

                    auto& pair = circleElements.at(uid);
                    auto& bcircle = pair.second;
                    if (circleAndBoxCollide(bcircle, queryBBox)) {
                        if (resultFn(pair.first, convertToBox(bcircle))) {
                            return;
                        }
                    }
                }
            }
        }
    }
}

template <class T>
void GridIndex<T>::query(const BCircle& queryBCircle, std::function<bool(const T&, const BBox&)> resultFn) const {
    std::unordered_set<size_t> seenBoxes;
    std::unordered_set<size_t> seenCircles;

    BBox queryBBox = convertToBox(queryBCircle);
    if (noIntersection(queryBBox)) {
        return;
    } else if (completeIntersection(queryBBox)) {
        for (auto& element : boxElements) {
            if (resultFn(element.first, element.second)) {
                return;
            }
        }
        for (auto& element : circleElements) {
            if (resultFn(element.first, convertToBox(element.second))) {
                return;
            }
        }
    }

    auto cx1 = convertToXCellCoord(queryBCircle.center.x - queryBCircle.radius);
    auto cy1 = convertToYCellCoord(queryBCircle.center.y - queryBCircle.radius);
    auto cx2 = convertToXCellCoord(queryBCircle.center.x + queryBCircle.radius);
    auto cy2 = convertToYCellCoord(queryBCircle.center.y + queryBCircle.radius);

    std::size_t x;
    std::size_t y;
    std::size_t cellIndex;
    for (x = cx1; x <= cx2; ++x) {
        for (y = cy1; y <= cy2; ++y) {
            cellIndex = xCellCount * y + x;
            // Look up boxes
            for (auto uid : boxCells[cellIndex]) {
                if (seenBoxes.count(uid) == 0) {
                    seenBoxes.insert(uid);

                    auto& pair = boxElements.at(uid);
                    auto& bbox = pair.second;
                    if (circleAndBoxCollide(queryBCircle, bbox)) {
                        if (resultFn(pair.first, bbox)) {
                            return;
                        }
                    }
                }
            }

            // Look up other circles
            for (auto uid : circleCells[cellIndex]) {
                if (seenCircles.count(uid) == 0) {
                    seenCircles.insert(uid);

                    auto& pair = circleElements.at(uid);
                    auto& bcircle = pair.second;
                    if (circlesCollide(queryBCircle, bcircle)) {
                        if (resultFn(pair.first, convertToBox(bcircle))) {
                            return;
                        }
                    }
                }
            }
        }
    }
}

template <class T>
std::size_t GridIndex<T>::convertToXCellCoord(const float x) const {
    return static_cast<size_t>(util::max(0.0, util::min(xCellCount - 1.0, std::floor(x * xScale))));
}

template <class T>
std::size_t GridIndex<T>::convertToYCellCoord(const float y) const {
    return static_cast<size_t>(util::max(0.0, util::min(yCellCount - 1.0, std::floor(y * yScale))));
}

template <class T>
bool GridIndex<T>::boxesCollide(const BBox& first, const BBox& second) const {
    return first.min.x <= second.max.x && first.min.y <= second.max.y && first.max.x >= second.min.x &&
           first.max.y >= second.min.y;
}

template <class T>
bool GridIndex<T>::circlesCollide(const BCircle& first, const BCircle& second) const {
    auto dx = second.center.x - first.center.x;
    auto dy = second.center.y - first.center.y;
    auto bothRadii = first.radius + second.radius;
    return (bothRadii * bothRadii) > (dx * dx + dy * dy);
}

template <class T>
bool GridIndex<T>::circleAndBoxCollide(const BCircle& circle, const BBox& box) const {
    auto halfRectWidth = (box.max.x - box.min.x) / 2;
    auto distX = std::abs(circle.center.x - (box.min.x + halfRectWidth));
    if (distX > (halfRectWidth + circle.radius)) {
        return false;
    }

    auto halfRectHeight = (box.max.y - box.min.y) / 2;
    auto distY = std::abs(circle.center.y - (box.min.y + halfRectHeight));
    if (distY > (halfRectHeight + circle.radius)) {
        return false;
    }

    if (distX <= halfRectWidth || distY <= halfRectHeight) {
        return true;
    }

    auto dx = distX - halfRectWidth;
    auto dy = distY - halfRectHeight;
    return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

template <class T>
bool GridIndex<T>::empty() const {
    return boxElements.empty() && circleElements.empty();
}

} // namespace mbgl
