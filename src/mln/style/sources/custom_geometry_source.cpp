#include <cstring>
#include <map>
#include <mln/actor/actor.hpp>
#include <mln/actor/scheduler.hpp>
#include <mln/style/custom_tile_loader.hpp>
#include <mln/style/layer.hpp>
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

Mutable<Source::Impl> CustomGeometrySource::createMutable() const noexcept {
    return staticMutableCast<Source::Impl>(makeMutable<Impl>(impl()));
}

} // namespace style
} // namespace mln
