#include <mln/actor/actor.hpp>
#include <mln/actor/scheduler.hpp>
#include <mln/style/custom_vector_tile_loader.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/source_observer.hpp>
#include <mln/style/sources/custom_vector_source.hpp>
#include <mln/style/sources/custom_vector_source_impl.hpp>
#include <mln/tile/tile.hpp>
#include <mln/tile/tile_id.hpp>

#include <utility>

namespace mln {
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

bool CustomVectorSource::supportsLayerType(const mln::style::LayerTypeInfo* info) const {
    return mln::underlying_type(Tile::Kind::Geometry) == mln::underlying_type(info->tileKind);
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
} // namespace mln
