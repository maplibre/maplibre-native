#pragma once

#include <mln/renderer/sources/render_tile_source.hpp>
#include <mln/renderer/tile_pyramid.hpp>
#include <mln/style/sources/tile_source_impl.hpp>

namespace mln {

class RenderVectorSource final : public RenderTileSetSource {
public:
    explicit RenderVectorSource(Immutable<style::TileSource::Impl>, const TaggedScheduler&);

    /// Enable the decoding of MLT tiles with FastPFOR integer encodings.
    /// Default is false.  Such tiles will fail if not explicitly enabled.
    void setFastPFOREnabled(bool enable) override { fastPFOREnabled = enable; }

protected:
    void updateInternal(const Tileset&,
                        const std::vector<Immutable<style::LayerProperties>>&,
                        bool needsRendering,
                        bool needsRelayout,
                        const TileParameters&) override;

    const std::optional<Tileset>& getTileset() const override;

private:
    std::optional<bool> isMLT;
    bool fastPFOREnabled = false;
};

} // namespace mln
