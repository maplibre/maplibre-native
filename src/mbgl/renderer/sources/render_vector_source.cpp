#include <mbgl/renderer/sources/render_vector_source.hpp>

#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/tile/vector_mlt_tile.hpp>
#include <mbgl/tile/vector_mvt_tile.hpp>

namespace mbgl {

using namespace style;

RenderVectorSource::RenderVectorSource(Immutable<style::TileSource::Impl> impl_, const TaggedScheduler& threadPool_)
    : RenderTileSetSource(std::move(impl_), threadPool_) {}

const std::optional<Tileset>& RenderVectorSource::getTileset() const {
    return static_cast<const style::TileSource::Impl&>(*baseImpl).tileset;
}

void RenderVectorSource::updateInternal(const Tileset& tileset,
                                        const std::vector<Immutable<style::LayerProperties>>& layers,
                                        const bool needsRendering,
                                        const bool needsRelayout,
                                        const TileParameters& parameters) {
    tilePyramid.update(
        layers,
        needsRendering,
        needsRelayout,
        parameters,
        *baseImpl,
        util::tileSize_I,
        tileset.zoomRange,
        tileset.bounds,
        [&](const OverscaledTileID& tileID, TileObserver* observer_) -> std::unique_ptr<VectorTile> {
            if (!isMLT.has_value()) {
                auto impl = staticImmutableCast<style::TileSource::Impl>(baseImpl);
                assert(impl->tileset); // we should have one by now
                if (impl->tileset) {
                    isMLT = (impl->tileset->vectorEncoding == Tileset::VectorEncoding::MLT);
                }
            }
            if (isMLT && *isMLT) {
                return std::make_unique<VectorMLTTile>(tileID, baseImpl->id, parameters, tileset, observer_);
            } else {
                return std::make_unique<VectorMVTTile>(tileID, baseImpl->id, parameters, tileset, observer_);
            }
        });
}

} // namespace mbgl
