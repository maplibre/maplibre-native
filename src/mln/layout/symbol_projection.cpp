#include <mln/layout/symbol_projection.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/renderer/layers/render_symbol_layer.hpp>
#include <mln/util/math.hpp>

#include <numbers>

using namespace std::numbers;

namespace mln {

/*
 * # Overview of coordinate spaces
 *
 * ## Tile coordinate spaces
 * Each label has an anchor. Some labels have corresponding line geometries.
 * The points for both anchors and lines are stored in tile units. Each tile has
 * it's own coordinate space going from (0, 0) at the top left to (EXTENT,
 * EXTENT) at the bottom right.
 *
 * ## GL coordinate space
 * At the end of everything, the vertex shader needs to produce a position in GL
 * coordinate space, which is (-1, 1) at the top left and (1, -1) in the bottom
 * right.
 *
 * ## Map pixel coordinate spaces
 * Each tile has a pixel coordinate space. It's just the tile units scaled so
 * that one unit is whatever counts as 1 pixel at the current zoom. This space
 * is used for pitch-alignment=map, rotation-alignment=map
 *
 * ## Rotated map pixel coordinate spaces
 * Like the above, but rotated so axis of the space are aligned with the
 * viewport instead of the tile. This space is used for pitch-alignment=map,
 * rotation-alignment=viewport
 *
 * ## Viewport pixel coordinate space
 * (0, 0) is at the top left of the canvas and (pixelWidth, pixelHeight) is at
 * the bottom right corner of the canvas. This space is used for
 * pitch-alignment=viewport
 *
 *
 * # Vertex projection
 * It goes roughly like this:
 * 1. project the anchor and line from tile units into the correct label
 * coordinate space
 *      - map pixel space           pitch-alignment=map rotation-alignment=map
 *      - rotated map pixel space   pitch-alignment=map
 * rotation-alignment=viewport
 *      - viewport pixel space      pitch-alignment=viewport
 * rotation-alignment=*
 * 2. if the label follows a line, find the point along the line that is the
 * correct distance from the anchor.
 * 3. add the glyph's corner offset to the point from step 3
 * 4. convert from the label coordinate space to gl coordinates
 *
 * For horizontal labels we want to do step 1 in the shader for performance
 * reasons (no cpu work). This is what `u_label_plane_matrix` is used for. For
 * labels aligned with lines we have to steps 1 and 2 on the cpu since we need
 * access to the line geometry. This is what `updateLineLabels(...)` in JS,
 * `reprojectLineLabels()` in gl-native, does. Since the conversion is handled
 * on the cpu we just set `u_label_plane_matrix` to an identity matrix.
 *
 * Steps 3 and 4 are done in the shaders for all labels.
 */

void getTileSkewVectors(const TransformState& state, vec2& vecEast, vec2& vecSouth) {
    const double cosRoll = cos(state.getRoll());
    const double sinRoll = sin(state.getRoll());
    const double cosPitch = cos(state.getPitch());
    const double cosBearing = cos(-state.getBearing());
    const double sinBearing = sin(-state.getBearing());
    vecSouth[0] = -cosBearing * cosPitch * sinRoll - sinBearing * cosRoll;
    vecSouth[1] = -sinBearing * cosPitch * sinRoll + cosBearing * cosRoll;
    const double vecSouthLen = std::hypot(vecSouth[0], vecSouth[1]);
    if (vecSouthLen < 1.0e-9) {
        vecSouth = {0, 0};
    } else {
        vecSouth[0] /= vecSouthLen;
        vecSouth[1] /= vecSouthLen;
    }
    vecEast[0] = cosBearing * cosPitch * cosRoll - sinBearing * sinRoll;
    vecEast[1] = sinBearing * cosPitch * cosRoll + cosBearing * sinRoll;
    const double vecEastLen = std::hypot(vecEast[0], vecEast[1]);
    if (vecEastLen < 1.0e-9) {
        vecEast = {0, 0};
    } else {
        vecEast[0] /= vecEastLen;
        vecEast[1] /= vecEastLen;
    }
}

mat4 getTileSkewMatrix(const TransformState& state) {
    vec2 vecEast, vecSouth;
    getTileSkewVectors(state, vecEast, vecSouth);
    mat4 m;
    matrix::identity(m);
    m[0] = vecEast[0];
    m[1] = vecEast[1];
    m[4] = vecSouth[0];
    m[5] = vecSouth[1];
    return m;
}

mat4 getLabelPlaneMatrix(const bool pitchWithMap,
                         const bool rotateWithMap,
                         const TransformState& state,
                         const float pixelsToTileUnits) {
    mat4 m;
    matrix::identity(m);
    if (pitchWithMap) {
        matrix::scale(m, m, 1 / pixelsToTileUnits, 1 / pixelsToTileUnits, 1);
        if (!rotateWithMap) {
            mat4 skew = getTileSkewMatrix(state);
            matrix::invert(skew, skew);
            matrix::multiply(m, m, skew);
        }
    } else {
        matrix::scale(m, m, state.getSize().width / 2.0, -(state.getSize().height / 2.0), 1.0);
        matrix::translate(m, m, 1, -1, 0);
    }
    return m;
}

mat4 getGlCoordMatrix(const bool pitchWithMap,
                      const bool rotateWithMap,
                      const TransformState& state,
                      const float pixelsToTileUnits) {
    mat4 m;
    matrix::identity(m);
    if (pitchWithMap) {
        matrix::scale(m, m, pixelsToTileUnits, pixelsToTileUnits, 1);
        if (!rotateWithMap) {
            matrix::multiply(m, m, getTileSkewMatrix(state));
        }
    } else {
        matrix::scale(m, m, 1, -1, 1);
        matrix::translate(m, m, -1, -1, 0);
        matrix::scale(m, m, 2.0 / state.getSize().width, 2.0 / state.getSize().height, 1.0);
    }
    return m;
}

LabelPlaneProjector::LabelPlaneProjector(const TileProjector& tile_,
                                         const bool pitchWithMap_,
                                         const bool rotateWithMap,
                                         const float pixelsToTileUnits,
                                         const Point<float> translation_)
    : tile(tile_),
      pitchWithMap(pitchWithMap_),
      translation(translation_),
      width(static_cast<float>(tile_.getTransformState().getSize().width)),
      height(static_cast<float>(tile_.getTransformState().getSize().height)) {
    // Only pitched labels lay out in the pitched plane; the others go through the projection.
    if (pitchWithMap) {
        pitchedLabelPlaneMatrix = getLabelPlaneMatrix(
            true, rotateWithMap, tile_.getTransformState(), pixelsToTileUnits);
        matrix::invert(pitchedLabelPlaneMatrixInverse, pitchedLabelPlaneMatrix);
    } else {
        matrix::identity(pitchedLabelPlaneMatrix);
        matrix::identity(pitchedLabelPlaneMatrixInverse);
    }
}

const ProjectedTilePoint& LineProjectionCache::get(std::size_t index,
                                                   const GeometryCoordinates& line,
                                                   const LabelPlaneProjector& labelPlane) {
    auto& cached = points[index];
    if (!cached) {
        cached = labelPlane.project(convertPoint<float>(line.at(index)));
        occluded |= cached->occluded;
    }
    return *cached;
}

ProjectedTilePoint LabelPlaneProjector::project(const Point<float>& tilePoint) const {
    const Point<float> translated = tilePoint + translation;
    if (pitchWithMap) {
        const auto projected = mln::project(translated, pitchedLabelPlaneMatrix);
        return {.point = {projected.first.x, projected.first.y},
                .signedDistanceFromCamera = projected.second,
                .occluded = false};
    }
    ProjectedTilePoint projected = tile.project({translated.x, translated.y});
    projected.point = {(projected.point.x * 0.5 + 0.5) * width, (-projected.point.y * 0.5 + 0.5) * height};
    return projected;
}

Point<float> LabelPlaneProjector::toClipSpace(const Point<float>& labelPlanePoint) const {
    if (pitchWithMap) {
        const auto tilePoint = mln::project(labelPlanePoint, pitchedLabelPlaneMatrixInverse).first;
        const auto projected = tile.project({tilePoint.x, tilePoint.y});
        return {static_cast<float>(projected.point.x), static_cast<float>(projected.point.y)};
    }
    return {(labelPlanePoint.x / width) * 2.0f - 1.0f, 1.0f - (labelPlanePoint.y / height) * 2.0f};
}

ProjectedTilePoint LabelPlaneProjector::toClipSpaceFromTile(const Point<float>& tilePoint) const {
    return tile.project({tilePoint.x, tilePoint.y});
}

PointAndCameraDistance project(const Point<float>& point, const mat4& matrix) {
    vec4 pos = {{point.x, point.y, 0, 1}};
    matrix::transformMat4(pos, pos, matrix);
    return {{static_cast<float>(pos[0] / pos[3]), static_cast<float>(pos[1] / pos[3])}, static_cast<float>(pos[3])};
}

float evaluateSizeForFeature(const ZoomEvaluatedSize& zoomEvaluatedSize, const PlacedSymbol& placedSymbol) {
    if (zoomEvaluatedSize.isFeatureConstant) {
        return zoomEvaluatedSize.size;
    } else {
        if (zoomEvaluatedSize.isZoomConstant) {
            return placedSymbol.lowerSize;
        } else {
            return placedSymbol.lowerSize + zoomEvaluatedSize.sizeT * (placedSymbol.upperSize - placedSymbol.lowerSize);
        }
    }
}

bool isVisible(const Point<double>& anchorPos, const std::array<double, 2>& clippingBuffer) {
    const double x = anchorPos.x;
    const double y = anchorPos.y;
    const bool inPaddedViewport = (x >= -clippingBuffer[0] && x <= clippingBuffer[0] && y >= -clippingBuffer[1] &&
                                   y <= clippingBuffer[1]);
    return inPaddedViewport;
}

void addDynamicAttributes(const Point<float>& anchorPoint,
                          const float angle,
                          SymbolBucket::DynamicAttributeVector& dynamicAttributeData) {
    auto dynamicAttributes = SymbolBucket::dynamicLayoutAttributes(anchorPoint, angle);
    dynamicAttributeData.emplace_back(dynamicAttributes);
#if !MLN_USE_SYMBOL_INSTANCING
    dynamicAttributeData.emplace_back(dynamicAttributes);
    dynamicAttributeData.emplace_back(dynamicAttributes);
    dynamicAttributeData.emplace_back(dynamicAttributes);
#endif
}

void hideGlyphs(size_t numGlyphs, SymbolBucket::DynamicAttributeVector& dynamicVertexArray) {
#if MLN_USE_SYMBOL_INSTANCING
    const size_t count = 1;
#else
    const size_t count = 4;
#endif
    const Point<float> offscreenPoint = {-INFINITY, -INFINITY};
    if (dynamicVertexArray.empty()) {
        dynamicVertexArray.reserve(count * numGlyphs);
    }
    for (size_t i = 0; i < numGlyphs; i++) {
        addDynamicAttributes(offscreenPoint, 0, dynamicVertexArray);
    }
}

enum PlacementResult {
    OK,
    NotEnoughRoom,
    NeedsFlipping,
    UseVertical
};

template <typename ProjectFn>
Point<float> projectTruncatedLineSegment(const Point<float>& previousTilePoint,
                                         const Point<float>& currentTilePoint,
                                         const Point<float>& previousProjectedPoint,
                                         const float minimumLength,
                                         const ProjectFn& projectPoint) {
    // We are assuming "previousTilePoint" won't project to a point within one
    // unit of the camera plane If it did, that would mean our label extended
    // all the way out from within the viewport to a (very distant) point near
    // the plane of the camera. We wouldn't be able to render the label anyway
    // once it crossed the plane of the camera.
    const Point<float> projectedUnitVertex = projectPoint(previousTilePoint +
                                                          util::unit<float>(previousTilePoint - currentTilePoint));
    const Point<float> projectedUnitSegment = previousProjectedPoint - projectedUnitVertex;

    return previousProjectedPoint + (projectedUnitSegment * (minimumLength / util::mag<float>(projectedUnitSegment)));
}

std::optional<PlacedGlyph> placeGlyphAlongLine(const float offsetX,
                                               const float lineOffsetX,
                                               const float lineOffsetY,
                                               const bool flip,
                                               const Point<float>& projectedAnchorPoint,
                                               const Point<float>& tileAnchorPoint,
                                               const uint16_t anchorSegment,
                                               const GeometryCoordinates& line,
                                               const std::vector<float>& tileDistances,
                                               const LabelPlaneProjector& labelPlane,
                                               LineProjectionCache& projections,
                                               const bool returnTileDistance) {
    const float combinedOffsetX = flip ? offsetX - lineOffsetX : offsetX + lineOffsetX;

    int16_t dir = combinedOffsetX > 0 ? 1 : -1;

    float angle = 0.0;
    if (flip) {
        // The label needs to be flipped to keep text upright.
        // Iterate in the reverse direction.
        dir *= -1;
        angle = pi_v<float>;
    }

    if (dir < 0) angle += pi_v<float>;

    int32_t currentIndex = dir > 0 ? anchorSegment : anchorSegment + 1;

    const int32_t initialIndex = currentIndex;
    Point<float> current = projectedAnchorPoint;
    Point<float> prev = projectedAnchorPoint;
    float distanceToPrev = 0.0;
    float currentSegmentDistance = 0.0;
    const float absOffsetX = std::abs(combinedOffsetX);

    while (distanceToPrev + currentSegmentDistance <= absOffsetX) {
        currentIndex += dir;

        // offset does not fit on the projected line
        if (currentIndex < 0 || std::cmp_greater_equal(currentIndex, static_cast<int32_t>(line.size()))) {
            return {};
        }

        prev = current;
        const ProjectedTilePoint& projection = projections.get(currentIndex, line, labelPlane);
        if (projection.signedDistanceFromCamera > 0) {
            current = {static_cast<float>(projection.point.x), static_cast<float>(projection.point.y)};
        } else {
            // The vertex is behind the plane of the camera, so we can't project it
            // Instead, we'll create a vertex along the line that's far enough to include the glyph
            const Point<float> previousTilePoint = distanceToPrev == 0
                                                       ? tileAnchorPoint
                                                       : convertPoint<float>(line.at(currentIndex - dir));
            const Point<float> currentTilePoint = convertPoint<float>(line.at(currentIndex));
            current = projectTruncatedLineSegment(
                previousTilePoint, currentTilePoint, prev, absOffsetX - distanceToPrev + 1, [&](const Point<float>& p) {
                    const auto projected = labelPlane.project(p);
                    return Point<float>{static_cast<float>(projected.point.x), static_cast<float>(projected.point.y)};
                });
        }

        distanceToPrev += currentSegmentDistance;
        currentSegmentDistance = util::dist<float>(prev, current);
    }

    // The point is on the current segment. Interpolate to find it.
    const float segmentInterpolationT = (absOffsetX - distanceToPrev) / currentSegmentDistance;
    const Point<float> prevToCurrent = current - prev;
    Point<float> p = (prevToCurrent * segmentInterpolationT) + prev;

    // offset the point from the line to text-offset and icon-offset
    p += util::perp(prevToCurrent) * static_cast<float>(lineOffsetY * dir / util::mag(prevToCurrent));

    const float segmentAngle = angle + std::atan2(current.y - prev.y, current.x - prev.x);

    return {{p,
             segmentAngle,
             returnTileDistance
                 ? TileDistance((currentIndex - dir) == initialIndex ? 0 : tileDistances[currentIndex - dir],
                                absOffsetX - distanceToPrev)
                 : std::optional<TileDistance>()}};
}

std::optional<std::pair<PlacedGlyph, PlacedGlyph>> placeFirstAndLastGlyph(const float fontScale,
                                                                          const float lineOffsetX,
                                                                          const float lineOffsetY,
                                                                          const bool flip,
                                                                          const Point<float>& anchorPoint,
                                                                          const Point<float>& tileAnchorPoint,
                                                                          const PlacedSymbol& symbol,
                                                                          const LabelPlaneProjector& labelPlane,
                                                                          LineProjectionCache& projections,
                                                                          const bool returnTileDistance) {
    if (symbol.glyphOffsets.empty()) {
        assert(false);
        return {};
    }

    const float firstGlyphOffset = symbol.glyphOffsets.front();
    const float lastGlyphOffset = symbol.glyphOffsets.back();

    std::optional<PlacedGlyph> firstPlacedGlyph = placeGlyphAlongLine(fontScale * firstGlyphOffset,
                                                                      lineOffsetX,
                                                                      lineOffsetY,
                                                                      flip,
                                                                      anchorPoint,
                                                                      tileAnchorPoint,
                                                                      static_cast<uint16_t>(symbol.segment),
                                                                      symbol.line,
                                                                      symbol.tileDistances,
                                                                      labelPlane,
                                                                      projections,
                                                                      returnTileDistance);
    if (!firstPlacedGlyph) return {};

    std::optional<PlacedGlyph> lastPlacedGlyph = placeGlyphAlongLine(fontScale * lastGlyphOffset,
                                                                     lineOffsetX,
                                                                     lineOffsetY,
                                                                     flip,
                                                                     anchorPoint,
                                                                     tileAnchorPoint,
                                                                     static_cast<uint16_t>(symbol.segment),
                                                                     symbol.line,
                                                                     symbol.tileDistances,
                                                                     labelPlane,
                                                                     projections,
                                                                     returnTileDistance);
    if (!lastPlacedGlyph) return {};
    // A line label with any vertex behind the planet is hidden whole.
    if (projections.anyOccluded()) return {};

    return std::make_pair(*firstPlacedGlyph, *lastPlacedGlyph);
}

std::optional<PlacementResult> requiresOrientationChange(const WritingModeType writingModes,
                                                         const Point<float>& firstPoint,
                                                         const Point<float>& lastPoint,
                                                         const float aspectRatio) {
    if (writingModes == (WritingModeType::Horizontal | WritingModeType::Vertical)) {
        // On top of choosing whether to flip, choose whether to render this
        // version of the glyphs or the alternate vertical glyphs. We can't just
        // filter out vertical glyphs in the horizontal range because the
        // horizontal and vertical versions can have slightly different
        // projections which could lead to angles where both or neither showed.
        auto rise = std::abs(lastPoint.y - firstPoint.y);
        auto run = std::abs(lastPoint.x - firstPoint.x) * aspectRatio;
        if (rise > run) {
            return PlacementResult::UseVertical;
        }
    }

    if ((writingModes == WritingModeType::Vertical) ? (firstPoint.y < lastPoint.y) : (firstPoint.x > lastPoint.x)) {
        // Includes "horizontalOnly" case for labels without vertical glyphs
        return PlacementResult::NeedsFlipping;
    }
    return {};
}

PlacementResult placeGlyphsAlongLine(const PlacedSymbol& symbol,
                                     const float fontSize,
                                     const bool flip,
                                     const bool keepUpright,
                                     const LabelPlaneProjector& labelPlane,
                                     LineProjectionCache& projections,
                                     SymbolBucket::DynamicAttributeVector& dynamicVertexArray,
                                     const Point<float>& projectedAnchorPoint,
                                     const float aspectRatio) {
    const float fontScale = fontSize / util::ONE_EM;
    const float lineOffsetX = symbol.lineOffset[0] * fontScale;
    const float lineOffsetY = symbol.lineOffset[1] * fontScale;

    std::vector<PlacedGlyph> placedGlyphs;
    if (symbol.glyphOffsets.size() > 1) {
        const std::optional<std::pair<PlacedGlyph, PlacedGlyph>> firstAndLastGlyph = placeFirstAndLastGlyph(
            fontScale,
            lineOffsetX,
            lineOffsetY,
            flip,
            projectedAnchorPoint,
            symbol.anchorPoint,
            symbol,
            labelPlane,
            projections,
            false);
        if (!firstAndLastGlyph) {
            return PlacementResult::NotEnoughRoom;
        }

        const Point<float> firstPoint = labelPlane.toClipSpace(firstAndLastGlyph->first.point);
        const Point<float> lastPoint = labelPlane.toClipSpace(firstAndLastGlyph->second.point);

        if (keepUpright && !flip) {
            auto orientationChange = requiresOrientationChange(symbol.writingModes, firstPoint, lastPoint, aspectRatio);
            if (orientationChange) {
                return *orientationChange;
            }
        }

        placedGlyphs.reserve(symbol.glyphOffsets.size());
        placedGlyphs.push_back(firstAndLastGlyph->first);
        for (size_t glyphIndex = 1; glyphIndex < symbol.glyphOffsets.size() - 1; glyphIndex++) {
            const float glyphOffsetX = symbol.glyphOffsets[glyphIndex];
            // Since first and last glyph fit on the line, we're sure that the
            // rest of the glyphs can be placed
            auto placedGlyph = placeGlyphAlongLine(glyphOffsetX * fontScale,
                                                   lineOffsetX,
                                                   lineOffsetY,
                                                   flip,
                                                   projectedAnchorPoint,
                                                   symbol.anchorPoint,
                                                   static_cast<uint16_t>(symbol.segment),
                                                   symbol.line,
                                                   symbol.tileDistances,
                                                   labelPlane,
                                                   projections,
                                                   false);
            if (placedGlyph) {
                placedGlyphs.push_back(*placedGlyph);
            } else {
                placedGlyphs.emplace_back(Point<float>{-INFINITY, -INFINITY}, 0.0f, std::nullopt);
            }
        }
        placedGlyphs.push_back(firstAndLastGlyph->second);
    } else if (symbol.glyphOffsets.size() == 1) {
        // Only a single glyph to place
        // So, determine whether to flip based on projected angle of the line segment it's on
        if (keepUpright && !flip) {
            const auto toClip = [&](const Point<float>& p) {
                const auto projected = labelPlane.toClipSpaceFromTile(p);
                return Point<float>{static_cast<float>(projected.point.x), static_cast<float>(projected.point.y)};
            };
            const Point<float> a = toClip(symbol.anchorPoint);
            const Point<float> tileSegmentEnd = convertPoint<float>(symbol.line.at(symbol.segment + 1));
            const ProjectedTilePoint projectedVertex = labelPlane.toClipSpaceFromTile(tileSegmentEnd);
            // We know the anchor will be in the viewport, but the end of the
            // line segment may be behind the plane of the camera, in which case
            // we can use a point at any arbitrary (closer) point on the
            // segment.
            const Point<float> b = (projectedVertex.signedDistanceFromCamera > 0)
                                       ? Point<float>{static_cast<float>(projectedVertex.point.x),
                                                      static_cast<float>(projectedVertex.point.y)}
                                       : projectTruncatedLineSegment(symbol.anchorPoint, tileSegmentEnd, a, 1, toClip);

            auto orientationChange = requiresOrientationChange(symbol.writingModes, a, b, aspectRatio);
            if (orientationChange) {
                return *orientationChange;
            }
        }
        const float glyphOffsetX = symbol.glyphOffsets.front();
        std::optional<PlacedGlyph> singleGlyph = placeGlyphAlongLine(fontScale * glyphOffsetX,
                                                                     lineOffsetX,
                                                                     lineOffsetY,
                                                                     flip,
                                                                     projectedAnchorPoint,
                                                                     symbol.anchorPoint,
                                                                     static_cast<uint16_t>(symbol.segment),
                                                                     symbol.line,
                                                                     symbol.tileDistances,
                                                                     labelPlane,
                                                                     projections,
                                                                     false);
        if (!singleGlyph || projections.anyOccluded()) return PlacementResult::NotEnoughRoom;

        placedGlyphs.push_back(*singleGlyph);
    }

    // The number of placedGlyphs must equal the number of glyphOffsets, which
    // must correspond to the number of glyph vertices There may be 0 glyphs
    // here, if a label consists entirely of glyphs that have 0x0 dimensions
    if (dynamicVertexArray.empty()) {
        dynamicVertexArray.reserve(4 * placedGlyphs.size());
    }
    for (auto& placedGlyph : placedGlyphs) {
        addDynamicAttributes(placedGlyph.point, placedGlyph.angle, dynamicVertexArray);
    }

    return PlacementResult::OK;
}

void reprojectLineLabels(SymbolBucket::DynamicAttributeVector& dynamicVertexArray,
                         const std::vector<PlacedSymbol>& placedSymbols,
                         const TileProjector& tileProjector,
                         bool pitchWithMap,
                         bool rotateWithMap,
                         bool keepUpright,
                         const RenderTile& tile,
                         const SymbolSizeBinder& sizeBinder,
                         const TransformState& state) {
    const ZoomEvaluatedSize partiallyEvaluatedSize = sizeBinder.evaluateForZoom(static_cast<float>(state.getZoom()));

    const std::array<double, 2> clippingBuffer = {
        {256.0 / state.getSize().width * 2.0 + 1.0, 256.0 / state.getSize().height * 2.0 + 1.0}};

    const float pixelsToTileUnits = tile.id.pixelsToTileUnits(1.0f, static_cast<float>(state.getZoom()));

    const LabelPlaneProjector labelPlane(tileProjector, pitchWithMap, rotateWithMap, pixelsToTileUnits);

    dynamicVertexArray.clear();

    bool useVertical = false;
    LineProjectionCache projections;

    for (auto& placedSymbol : placedSymbols) {
        // Don't do calculations for vertical glyphs unless the previous symbol
        // was horizontal and we determined that vertical glyphs were necessary.
        // Also don't do calculations for symbols that are collided and fully
        // faded out
        if (placedSymbol.hidden || (placedSymbol.writingModes == WritingModeType::Vertical && !useVertical)) {
            hideGlyphs(placedSymbol.glyphOffsets.size(), dynamicVertexArray);
            continue;
        }
        // Awkward... but we're counting on the paired "vertical" symbol coming
        // immediately after its horizontal counterpart
        useVertical = false;

        const ProjectedTilePoint anchorPos = tileProjector.project(
            {placedSymbol.anchorPoint.x, placedSymbol.anchorPoint.y});

        // Don't bother calculating the correct point for invisible labels.
        if (anchorPos.occluded || !isVisible(anchorPos.point, clippingBuffer)) {
            hideGlyphs(placedSymbol.glyphOffsets.size(), dynamicVertexArray);
            continue;
        }

        const auto cameraToAnchorDistance = static_cast<float>(anchorPos.signedDistanceFromCamera);
        const float perspectiveRatio = 0.5f + 0.5f * (cameraToAnchorDistance / state.getCameraToCenterDistance());

        const float fontSize = evaluateSizeForFeature(partiallyEvaluatedSize, placedSymbol);
        const float pitchScaledFontSize = pitchWithMap
                                              ? fontSize * perspectiveRatio *
                                                    static_cast<float>(tileProjector.pitchedTextCorrection(
                                                        {placedSymbol.anchorPoint.x, placedSymbol.anchorPoint.y}))
                                              : fontSize / perspectiveRatio;

        const auto projectedAnchor = labelPlane.project(placedSymbol.anchorPoint).point;
        const Point<float> anchorPoint{static_cast<float>(projectedAnchor.x), static_cast<float>(projectedAnchor.y)};

        projections.reset(placedSymbol.line.size());
        PlacementResult placeUnflipped = placeGlyphsAlongLine(placedSymbol,
                                                              pitchScaledFontSize,
                                                              false /*unflipped*/,
                                                              keepUpright,
                                                              labelPlane,
                                                              projections,
                                                              dynamicVertexArray,
                                                              anchorPoint,
                                                              state.getSize().aspectRatio());

        useVertical = placeUnflipped == PlacementResult::UseVertical;

        if (placeUnflipped == PlacementResult::NotEnoughRoom || useVertical ||
            (placeUnflipped == PlacementResult::NeedsFlipping &&
             placeGlyphsAlongLine(placedSymbol,
                                  pitchScaledFontSize,
                                  true /*flipped*/,
                                  keepUpright,
                                  labelPlane,
                                  projections,
                                  dynamicVertexArray,
                                  anchorPoint,
                                  state.getSize().aspectRatio()) == PlacementResult::NotEnoughRoom)) {
            hideGlyphs(placedSymbol.glyphOffsets.size(), dynamicVertexArray);
        }
    }
}
} // end namespace mln
