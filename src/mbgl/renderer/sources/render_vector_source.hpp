#pragma once

#include <mbgl/renderer/sources/render_tile_source.hpp>
#include <mbgl/renderer/tile_pyramid.hpp>
#include <mbgl/style/sources/tile_source_impl.hpp>

namespace mbgl {

class RenderVectorSource final : public RenderTileSetSource {
public:
    explicit RenderVectorSource(Immutable<style::TileSource::Impl>, const TaggedScheduler&);

private:
    void updateInternal(const Tileset&,
                        const std::vector<Immutable<style::LayerProperties>>&,
                        bool needsRendering,
                        bool needsRelayout,
                        const TileParameters&) override;
    const std::optional<Tileset>& getTileset() const override;

private:
    std::optional<bool> isMLT;
};

} // namespace mbgl
