#pragma once

#include <mln/renderer/sources/render_tile_source.hpp>
#include <mln/renderer/tile_pyramid.hpp>
#include <mln/style/sources/tile_source_impl.hpp>

namespace mln {

class RenderVectorSource final : public RenderTileSetSource {
public:
    explicit RenderVectorSource(Immutable<style::TileSource::Impl>, const TaggedScheduler&);

protected:
    void updateInternal(const Tileset&,
                        const std::vector<Immutable<style::LayerProperties>>&,
                        bool needsRendering,
                        bool needsRelayout,
                        const TileParameters&) override;

    const std::optional<Tileset>& getTileset() const override;

private:
    std::optional<bool> isMLT;
};

} // namespace mln
