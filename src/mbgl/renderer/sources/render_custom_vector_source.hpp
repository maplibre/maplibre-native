#pragma once

#include <mbgl/renderer/sources/render_tile_source.hpp>
#include <mbgl/style/sources/custom_vector_source_impl.hpp>

namespace mbgl {

class RenderCustomVectorSource final : public RenderTileSource {
public:
    explicit RenderCustomVectorSource(Immutable<style::CustomVectorSource::Impl>, const TaggedScheduler&);

    void update(Immutable<style::Source::Impl>,
                const std::vector<Immutable<style::LayerProperties>>&,
                bool needsRendering,
                bool needsRelayout,
                const TileParameters&) override;

private:
    const style::CustomVectorSource::Impl& impl() const;
};

} // namespace mbgl
