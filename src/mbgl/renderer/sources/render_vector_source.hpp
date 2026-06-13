#pragma once

#include <mbgl/renderer/sources/render_tile_source.hpp>
#include <mbgl/renderer/tile_pyramid.hpp>
#include <mbgl/style/sources/tile_source_impl.hpp>

namespace mbgl {

class RenderVectorSource final : public RenderTileSetSource {
public:
    explicit RenderVectorSource(Immutable<style::TileSource::Impl>, const TaggedScheduler&);

    /// Enable the decoding of MLT tiles with FastPFOR integer encodings.
    /// Default is false.  Such tiles will fail if not explicitly enabled.
    void setEnableFastPFOR(bool enable) { mltSupportsFastPFOR = enable; }

private:
    void updateInternal(const Tileset&,
                        const std::vector<Immutable<style::LayerProperties>>&,
                        bool needsRendering,
                        bool needsRelayout,
                        const TileParameters&) override;
    const std::optional<Tileset>& getTileset() const override;

private:
    std::optional<bool> isMLT;
    bool mltSupportsFastPFOR = false;
};

} // namespace mbgl
