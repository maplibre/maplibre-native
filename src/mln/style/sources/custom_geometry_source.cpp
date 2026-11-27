#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <map>
#include <mln/actor/actor.hpp>
#include <mln/actor/scheduler.hpp>
#include <mln/map/world_coordinate_helper.hpp>
#include <mln/style/custom_tile_loader.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/projection_impl.hpp>
#include <mln/style/source_observer.hpp>
#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/style/sources/custom_geometry_source_impl.hpp>
#include <mln/tile/tile.hpp>
#include <mln/tile/tile_id.hpp>
#include <tuple>

namespace mln {
namespace style {

CustomGeometrySource::CustomGeometrySource(std::string id, const CustomGeometrySource::Options& options)
    : Source(makeMutable<CustomGeometrySource::Impl>(std::move(id), options)),
      loader(std::make_unique<Actor<CustomTileLoader>>(
          Scheduler::GetBackground(), options.fetchTileFunction, options.cancelTileFunction)) {}

CustomGeometrySource::~CustomGeometrySource() = default;

const CustomGeometrySource::Impl& CustomGeometrySource::impl() const {
    return static_cast<const CustomGeometrySource::Impl&>(*baseImpl);
}

void CustomGeometrySource::loadDescription(FileSource&) {
    baseImpl = makeMutable<Impl>(impl(), loader->self());
    loaded = true;
    observer->onSourceLoaded(*this);
}

bool CustomGeometrySource::supportsLayerType(const mln::style::LayerTypeInfo* info) const {
    return mln::underlying_type(Tile::Kind::Geometry) == mln::underlying_type(info->tileKind);
}

void CustomGeometrySource::setTileData(const CanonicalTileID& tileID, const GeoJSON& data) {
    loader->self().invoke(&CustomTileLoader::setTileData, tileID, data);
}

void CustomGeometrySource::invalidateTile(const CanonicalTileID& tileID) {
    loader->self().invoke(&CustomTileLoader::invalidateTile, tileID);
}

void CustomGeometrySource::invalidateRegion(const LatLngBounds& bounds) {
    loader->self().invoke(&CustomTileLoader::invalidateRegion, bounds, impl().getZoomRange());
}

LatLngBounds CustomGeometrySource::getTileBounds(const CanonicalTileID& tileID) const {
    std::shared_ptr<const CRSWorldCoordinateHelper> registeredProjection;
    {
        std::scoped_lock lock(projectionMutex);
        if (projection) {
            registeredProjection = (*projection)->registeredProjection;
        }
    }
    if (!registeredProjection) {
        return LatLngBounds(tileID);
    }

    const double worldSize = std::pow(2.0, tileID.z);
    const double minX = tileID.x / worldSize;
    const double minY = tileID.y / worldSize;
    const double maxX = (tileID.x + 1.0) / worldSize;
    const double maxY = (tileID.y + 1.0) / worldSize;
    const std::array<std::array<double, 2>, 4> corners{{{minX, minY}, {maxX, minY}, {maxX, maxY}, {minX, maxY}}};
    std::array<std::array<double, 2>, 4> lngLats{};
    registeredProjection->lngLatsFromWorld(corners, lngLats);
    const auto lngs = std::ranges::minmax(lngLats, {}, [](const auto& lngLat) { return lngLat[0]; });
    const auto lats = std::ranges::minmax(lngLats, {}, [](const auto& lngLat) { return lngLat[1]; });
    return LatLngBounds::hull({lats.min[1], lngs.min[0]}, {lats.max[1], lngs.max[0]});
}

void CustomGeometrySource::setProjection(Immutable<Projection::Impl> projection_) {
    std::scoped_lock lock(projectionMutex);
    projection = std::move(projection_);
}

Mutable<Source::Impl> CustomGeometrySource::createMutable() const noexcept {
    return staticMutableCast<Source::Impl>(makeMutable<Impl>(impl()));
}

} // namespace style
} // namespace mln
