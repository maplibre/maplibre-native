#pragma once

#include <mbgl/gfx/texture.hpp>
#include <mbgl/renderer/image_atlas.hpp>
#include <mbgl/style/layer_impl.hpp>

#include <memory>
#include <optional>

namespace mbgl {

namespace gfx {
#if MLN_DRAWABLE_RENDERER
class Texture2D;
using Texture2DPtr = std::shared_ptr<gfx::Texture2D>;
#endif

class UploadPass;
} // namespace gfx

class Bucket;
class LayerRenderData;
class SourcePrepareParameters;

class TileAtlasTextures {
public:
#if MLN_DRAWABLE_RENDERER
    gfx::Texture2DPtr glyph;
    gfx::Texture2DPtr icon;
#else
    std::optional<gfx::Texture> glyph;
    std::optional<gfx::Texture> icon;
#endif
};

class TileRenderData {
public:
    virtual ~TileRenderData();

#if MLN_DRAWABLE_RENDERER
    const gfx::Texture2DPtr& getGlyphAtlasTexture() const;
    const gfx::Texture2DPtr& getIconAtlasTexture() const;
#else
    const gfx::Texture& getGlyphAtlasTexture() const;
    const gfx::Texture& getIconAtlasTexture() const;
#endif

    const std::shared_ptr<TileAtlasTextures>& getAtlasTextures() const { return atlasTextures; }
    // To be implemented for concrete tile types.
    virtual std::optional<ImagePosition> getPattern(const std::string&) const;
    virtual const LayerRenderData* getLayerRenderData(const style::Layer::Impl&) const;
    virtual Bucket* getBucket(const style::Layer::Impl&) const;
    virtual void upload(gfx::UploadPass&) {}
    virtual void prepare(const SourcePrepareParameters&) {}

protected:
    TileRenderData();
    TileRenderData(std::shared_ptr<TileAtlasTextures>);
    std::shared_ptr<TileAtlasTextures> atlasTextures;
};

template <typename BucketType>
class SharedBucketTileRenderData final : public TileRenderData {
public:
    SharedBucketTileRenderData(std::shared_ptr<BucketType> bucket_)
        : bucket(std::move(bucket_)) {}

private:
    // TileRenderData overrides.
    Bucket* getBucket(const style::Layer::Impl&) const override { return bucket ? bucket.get() : nullptr; }
    void upload(gfx::UploadPass& uploadPass) override {
        if (bucket) bucket->upload(uploadPass);
    }

    std::shared_ptr<BucketType> bucket;
};

} // namespace mbgl
