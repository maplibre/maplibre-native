#pragma once

#include <mln/renderer/sources/render_tile_source.hpp>
#include <mln/style/sources/custom_geometry_source_impl.hpp>

namespace mln {

class RenderCustomGeometrySource final : public RenderTileSource {
public:
    explicit RenderCustomGeometrySource(Immutable<style::CustomGeometrySource::Impl>, const TaggedScheduler&);

    void update(Immutable<style::Source::Impl>,
                const std::vector<Immutable<style::LayerProperties>>&,
                bool needsRendering,
                bool needsRelayout,
                const TileParameters&) override;

private:
    const style::CustomGeometrySource::Impl& impl() const;
};

} // namespace mln
