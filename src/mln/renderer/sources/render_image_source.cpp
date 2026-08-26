#include <mln/map/transform_state.hpp>
#include <mln/math/log2.hpp>
#include <mln/renderer/buckets/raster_bucket.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/sources/render_image_source.hpp>
#include <mln/renderer/tile_parameters.hpp>
#include <mln/renderer/render_static_data.hpp>
#include <mln/gfx/cull_face_mode.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/instrumentation.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/subdivision_granularity.hpp>
#include <mln/util/tile_coordinate.hpp>
#include <mln/util/tile_cover.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace mln {

using namespace style;

namespace {

// The mapping from the unit square onto the image's corners: the perspective terms of the
// projective warp and how far it is blended towards the bilinear one. Port of GL JS
// `calculateImageWarp` for the "auto" warp, after Heckbert, "Fundamentals of Texture Mapping and
// Image Warping" (1989), 2.2.3 and A.2.
struct ImageWarp {
    double perspectiveX = 0;
    double perspectiveY = 0;
    double blend = 1;
};

ImageWarp imageWarp(const GeometryCoordinates& corners) {
    constexpr double projectiveForeshortening = 4;
    constexpr double bilinearForeshortening = 512;
    const auto& topLeft = corners[0];
    const auto& topRight = corners[1];
    const auto& bottomRight = corners[2];
    const auto& bottomLeft = corners[3];
    // A parallelogram is warped affinely either way.
    if (topLeft.x + bottomRight.x == topRight.x + bottomLeft.x &&
        topLeft.y + bottomRight.y == topRight.y + bottomLeft.y) {
        return {};
    }
    const double sumX = topLeft.x - topRight.x + bottomRight.x - bottomLeft.x;
    const double sumY = topLeft.y - topRight.y + bottomRight.y - bottomLeft.y;
    const double rightX = topRight.x - bottomRight.x;
    const double rightY = topRight.y - bottomRight.y;
    const double downX = bottomLeft.x - bottomRight.x;
    const double downY = bottomLeft.y - bottomRight.y;
    const double determinant = rightX * downY - rightY * downX;
    const double perspectiveX = (sumX * downY - downX * sumY) / determinant;
    const double perspectiveY = (rightX * sumY - sumX * rightY) / determinant;
    // The homogeneous denominator at the four corners, normalized to one at the top left; its spread
    // is the foreshortening, and it stays finite and positive only while the quad is a perspective
    // view of the image.
    const double denominators[] = {1, 1 + perspectiveX, 1 + perspectiveX + perspectiveY, 1 + perspectiveY};
    const double foreshortening = *std::max_element(std::begin(denominators), std::end(denominators)) /
                                  *std::min_element(std::begin(denominators), std::end(denominators));
    const double blend = std::max(
        0.0, (1 - projectiveForeshortening / foreshortening) / (1 - projectiveForeshortening / bilinearForeshortening));
    if (!(foreshortening >= 1 && foreshortening <= bilinearForeshortening) || blend >= 1) {
        return {};
    }
    return {.perspectiveX = perspectiveX, .perspectiveY = perspectiveY, .blend = blend};
}

// Where the unit-square point (u, v) lands on the corners; the same blend GL JS's raster vertex
// shader evaluates per vertex.
Point<double> warpPosition(const GeometryCoordinates& corners, const ImageWarp& warp, double u, double v) {
    const Point<double> topLeft = convertPoint<double>(corners[0]);
    const Point<double> topRight = convertPoint<double>(corners[1]);
    const Point<double> bottomRight = convertPoint<double>(corners[2]);
    const Point<double> bottomLeft = convertPoint<double>(corners[3]);
    const Point<double> bilinear = util::interpolate(
        util::interpolate(topLeft, topRight, u), util::interpolate(bottomLeft, bottomRight, u), v);
    const double denominator = warp.perspectiveX * u + warp.perspectiveY * v + 1;
    const Point<double> acrossTop = topRight - topLeft + topRight * warp.perspectiveX;
    const Point<double> downLeft = bottomLeft - topLeft + bottomLeft * warp.perspectiveY;
    const Point<double> projective = (acrossTop * u + downLeft * v + topLeft) / denominator;
    return util::interpolate(projective, bilinear, warp.blend);
}

int16_t toVertexCoordinate(double value) {
    return static_cast<int16_t>(util::clamp<double>(
        std::round(value), std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max()));
}

// The image quad as a grid of `cells` × `cells`, textured edge to edge; one cell is the flat quad.
void buildImageMesh(RasterBucket& bucket, const GeometryCoordinates& corners, uint32_t cells) {
    const ImageWarp warp = imageWarp(corners);
    const uint32_t rows = cells + 1;
    for (uint32_t j = 0; j < rows; ++j) {
        for (uint32_t i = 0; i < rows; ++i) {
            const double u = static_cast<double>(i) / cells;
            const double v = static_cast<double>(j) / cells;
            const Point<double> position = warpPosition(corners, warp, u, v);
            bucket.vertices.emplace_back(
                RasterBucket::layoutVertex({toVertexCoordinate(position.x), toVertexCoordinate(position.y)},
                                           {static_cast<uint16_t>(std::lround(u * util::EXTENT)),
                                            static_cast<uint16_t>(std::lround(v * util::EXTENT))}));
        }
    }
    for (uint32_t j = 0; j < cells; ++j) {
        for (uint32_t i = 0; i < cells; ++i) {
            const auto topLeft = static_cast<uint16_t>(j * rows + i);
            const auto topRight = static_cast<uint16_t>(topLeft + 1);
            const auto bottomLeft = static_cast<uint16_t>(topLeft + rows);
            const auto bottomRight = static_cast<uint16_t>(bottomLeft + 1);
            bucket.indices.emplace_back(topLeft, topRight, bottomLeft);
            bucket.indices.emplace_back(topRight, bottomLeft, bottomRight);
        }
    }
    bucket.segments.emplace_back(0, 0, bucket.vertices.elements(), bucket.indices.elements());
    bucket.vertices.updateModified();
}

} // namespace

ImageSourceRenderData::~ImageSourceRenderData() = default;

void ImageSourceRenderData::upload(gfx::UploadPass& uploadPass) const {
    if (bucket && bucket->needsUpload()) {
        bucket->upload(uploadPass);
    }
}

RenderImageSource::RenderImageSource(Immutable<style::ImageSource::Impl> impl_)
    : RenderSource(std::move(impl_)) {}

RenderImageSource::~RenderImageSource() = default;

const style::ImageSource::Impl& RenderImageSource::impl() const {
    return static_cast<const style::ImageSource::Impl&>(*baseImpl);
}

bool RenderImageSource::isLoaded() const {
    return !!bucket;
}

std::unique_ptr<RenderItem> RenderImageSource::createRenderItem() {
    assert(renderData);
    return std::move(renderData);
}

void RenderImageSource::prepare(const SourcePrepareParameters&) {
    MLN_TRACE_FUNC();
    assert(!renderData);
    renderData = std::make_unique<ImageSourceRenderData>(
        bucket, isLoaded() ? tileIds : std::vector<UnwrappedTileID>{}, baseImpl->id);
}

std::unordered_map<std::string, std::vector<Feature>> RenderImageSource::queryRenderedFeatures(
    const ScreenLineString&,
    const TransformState&,
    const std::unordered_map<std::string, const RenderLayer*>&,
    const RenderedQueryOptions&,
    const mat4&) const {
    return std::unordered_map<std::string, std::vector<Feature>>{};
}

std::vector<Feature> RenderImageSource::querySourceFeatures(const SourceQueryOptions&) const {
    return {};
}

void RenderImageSource::update(Immutable<style::Source::Impl> baseImpl_,
                               const std::vector<Immutable<LayerProperties>>&,
                               const bool needsRendering,
                               const bool,
                               const TileParameters& parameters) {
    enabled = needsRendering;
    if (!needsRendering) {
        return;
    }

    auto transformState = parameters.transformState;
    std::swap(baseImpl, baseImpl_);

    auto coords = impl().getCoordinates();
    std::shared_ptr<PremultipliedImage> image = impl().getImage();

    if (!image || !image->valid()) {
        enabled = false;
        return;
    }

    // Compute the z0 tile coordinates for the given LatLngs
    TileCoordinatePoint nePoint = {-INFINITY, -INFINITY};
    TileCoordinatePoint swPoint = {INFINITY, INFINITY};
    std::vector<TileCoordinatePoint> tileCoordinates;
    for (LatLng latLng : coords) {
        auto point = TileCoordinate::fromLatLng(0, latLng).p;
        tileCoordinates.push_back(point);
        swPoint.x = std::min(swPoint.x, point.x);
        nePoint.x = std::max(nePoint.x, point.x);
        swPoint.y = std::min(swPoint.y, point.y);
        nePoint.y = std::max(nePoint.y, point.y);
    }

    // Calculate the optimum zoom level to determine the tile ids to use for transforms
    const auto dx = nePoint.x - swPoint.x;
    const auto dy = nePoint.y - swPoint.y;
    const auto dMax = std::max(dx, dy);
    const auto zoom = static_cast<uint8_t>(std::max(0.0, std::floor(-util::log2(dMax))));

    // Only enable if the long side of the image is > 2 pixels. Resulting in a
    // display of at least 2 x 1 px image
    // A tile coordinate unit represents the length of one tile (tileSize) at a given zoom.
    // To convert a tile coordinate to pixels, multiply by tileSize.
    // Here dMax is in z0 tile units, so we also scale by 2^z to match current zoom.
    enabled = dMax * std::pow(2.0, transformState.getZoom()) * util::tileSize_D > 2.0;
    if (!enabled) {
        return;
    }

    auto imageBounds = LatLngBounds::hull(coords[0], coords[1]);
    imageBounds.extend(coords[2]);
    imageBounds.extend(coords[3]);
    auto tileCover = util::tileCover(imageBounds, zoom);
    tileIds.clear();
    tileIds.push_back(tileCover[0]);

    bool hasVisibleTile = false;
    // Add additional wrapped tile ids if necessary
    Range<uint8_t> zoomRange(0, zoom);
    auto idealTiles = util::tileCover({.transformState = transformState,
                                       .tileLodMinRadius = parameters.tileLodMinRadius,
                                       .tileLodScale = parameters.tileLodScale,
                                       .tileLodPitchThreshold = parameters.tileLodPitchThreshold,
                                       .tileLodMode = parameters.tileLodMode},
                                      transformState.getIntegerZoom(),
                                      zoomRange);
    for (auto tile : idealTiles) {
        for (auto coveringTile : tileCover) {
            if (coveringTile.canonical == tile.canonical || coveringTile.canonical.isChildOf(tile.canonical) ||
                tile.canonical.isChildOf(coveringTile.canonical)) {
                hasVisibleTile = true;
                const UnwrappedTileID wrappedTileID(tile.wrap, tileCover[0].canonical);
                if (tile.wrap != 0 && std::find(tileIds.cbegin(), tileIds.cend(), wrappedTileID) == tileIds.cend()) {
                    tileIds.push_back(wrappedTileID);
                }
                break;
            }
        }
    }

    enabled = hasVisibleTile;
    if (!enabled) {
        return;
    }

    // Calculate Geometry Coordinates based on tile cover at ideal zoom
    GeometryCoordinates geomCoords;
    for (auto tileCoords : tileCoordinates) {
        auto gc = TileCoordinate::toGeometryCoordinate(tileIds[0], tileCoords);
        geomCoords.push_back(gc);
    }
    if (!bucket) {
        bucket = std::make_shared<RasterBucket>(image);
    } else if (image != bucket->image) {
        bucket->setImage(image);
    }

    // The globe bends the quad over the sphere with the tile mesh granularity of its tile, like GL JS.
    const uint32_t cells = transformState.isGlobeRendering()
                               ? SubdivisionGranularitySetting::globe().tile.getGranularityForZoomLevel(
                                     tileIds[0].canonical.z)
                               : 1;
    if (bucket->vertices.empty() || geomCoords != meshCorners || cells != meshCells) {
        bucket->clear();
        buildImageMesh(*bucket, geomCoords, cells);
        meshCorners = geomCoords;
        meshCells = cells;
    }
}

void RenderImageSource::dumpDebugLogs() const {
    Log::Info(Event::General, "RenderImageSource::id: " + impl().id);
    Log::Info(Event::General, "RenderImageSource::loaded: " + std::string(isLoaded() ? "yes" : "no"));
}

} // namespace mln
