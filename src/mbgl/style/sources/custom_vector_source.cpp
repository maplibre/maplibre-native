#include <mbgl/actor/actor.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/style/custom_vector_tile_loader.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/source_observer.hpp>
#include <mbgl/style/sources/custom_vector_source.hpp>
#include <mbgl/style/sources/custom_vector_source_impl.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/tile/tile_id.hpp>

#include <utility>

namespace mbgl {
namespace style {

CustomVectorSource::CustomVectorSource(std::string id, const CustomVectorSource::Options& options)
    : Source(makeMutable<CustomVectorSource::Impl>(std::move(id), options)),
      loader(std::make_unique<Actor<CustomVectorTileLoader>>(
          Scheduler::GetBackground(), options.fetchTileFunction, options.cancelTileFunction)) {}

CustomVectorSource::~CustomVectorSource() = default;

const CustomVectorSource::Impl& CustomVectorSource::impl() const {
    return static_cast<const CustomVectorSource::Impl&>(*baseImpl);
}

void CustomVectorSource::loadDescription(FileSource&) {
    baseImpl = makeMutable<Impl>(impl(), loader->self());
    loaded = true;
    observer->onSourceLoaded(*this);
}

bool CustomVectorSource::supportsLayerType(const mbgl::style::LayerTypeInfo* info) const {
    return mbgl::underlying_type(Tile::Kind::Geometry) == mbgl::underlying_type(info->tileKind);
}

void CustomVectorSource::setTileData(const CanonicalTileID& tileID,
                                     const std::shared_ptr<const std::string>& data,
                                     TileDataFormat format) {
    loader->self().invoke(&CustomVectorTileLoader::setTileData, tileID, data, format);
}

void CustomVectorSource::setTileError(const CanonicalTileID& tileID, std::exception_ptr error) {
    loader->self().invoke(&CustomVectorTileLoader::setTileError, tileID, error);
}

void CustomVectorSource::invalidateTile(const CanonicalTileID& tileID) {
    loader->self().invoke(&CustomVectorTileLoader::invalidateTile, tileID);
}

Mutable<Source::Impl> CustomVectorSource::createMutable() const noexcept {
    return staticMutableCast<Source::Impl>(makeMutable<Impl>(impl()));
}

} // namespace style
} // namespace mbgl
