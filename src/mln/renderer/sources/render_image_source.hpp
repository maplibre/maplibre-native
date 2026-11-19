#pragma once

#include <mln/renderer/render_source.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/render_tree.hpp>
#include <mln/style/sources/image_source_impl.hpp>

namespace mln {

class RasterBucket;

class ImageSourceRenderData final : public RenderItem {
public:
    ImageSourceRenderData(std::shared_ptr<RasterBucket> bucket_,
                          std::vector<UnwrappedTileID> tileIds_,
                          std::vector<UnwrappedTileID> cover_,
                          std::string name_)
        : bucket(std::move(bucket_)),
          tileIds(std::move(tileIds_)),
          cover(std::move(cover_)),
          name(std::move(name_)) {}
    ~ImageSourceRenderData() override;
    const std::shared_ptr<RasterBucket> bucket;
    /// The tiles the image is drawn in (the bucket's vertices are in the first one's coordinates)
    /// and the same image's covering tiles, which tileIds[0] is the first of.
    const std::vector<UnwrappedTileID> tileIds;
    const std::vector<UnwrappedTileID> cover;

private:
    void upload(gfx::UploadPass&) const override;
    void render(PaintParameters&) const override {}
    bool hasRenderPass(RenderPass) const override { return false; }
    const std::string& getName() const override { return name; }
    std::string name;
    void updateDebugDrawables(DebugLayerGroupMap&, PaintParameters&) const override {};
};

class RenderImageSource final : public RenderSource {
public:
    explicit RenderImageSource(Immutable<style::ImageSource::Impl>);
    ~RenderImageSource() override;

    bool isLoaded() const final;

    std::unique_ptr<RenderItem> createRenderItem() override;
    void prepare(const SourcePrepareParameters&) final;
    void updateFadingTiles() final {}
    bool hasFadingTiles() const final { return false; }

    void update(Immutable<style::Source::Impl>,
                const std::vector<Immutable<style::LayerProperties>>&,
                bool needsRendering,
                bool needsRelayout,
                const TileParameters&) final;

    const ImageSourceRenderData* getImageRenderData() const override { return renderData.get(); }

    std::unordered_map<std::string, std::vector<Feature>> queryRenderedFeatures(
        const ScreenLineString& geometry,
        const TransformState& transformState,
        const std::unordered_map<std::string, const RenderLayer*>& layers,
        const RenderedQueryOptions& options,
        const GlobalStateMap* globalState,
        const mat4& projMatrix) const final;

    std::vector<Feature> querySourceFeatures(const SourceQueryOptions&, const GlobalStateMap*) const final;

    void reduceMemoryUse() final {}
    void dumpDebugLogs() const final;

private:
    friend class RenderRasterLayer;
    const style::ImageSource::Impl& impl() const;

    std::shared_ptr<RasterBucket> bucket;
    std::unique_ptr<ImageSourceRenderData> renderData;
    std::vector<UnwrappedTileID> tileIds;
    std::vector<UnwrappedTileID> cover;
};

} // namespace mln
