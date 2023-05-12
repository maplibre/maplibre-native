#pragma once

#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/identity.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace mbgl {
class LayerGroup;
class PaintParameters;
class RenderOrchestrator;
class TileLayerGroup;

using LayerGroupPtr = std::shared_ptr<LayerGroup>;

namespace gfx {
class Context;
class Drawable;
class UploadPass;

using DrawablePtr = std::shared_ptr<Drawable>;
using UniqueDrawable = std::unique_ptr<Drawable>;
} // namespace gfx

/**
    A layer-like group of drawables, not a group of layers.
 */
class LayerGroup : public util::SimpleIdentifiable {
protected:
    LayerGroup(int32_t layerIndex);

public:
    LayerGroup(const LayerGroup&) = delete;
    LayerGroup& operator=(const LayerGroup&) = delete;

    int32_t getLayerIndex() const { return layerIndex; }

    /// Called before starting each frame
    virtual void preRender(RenderOrchestrator&, PaintParameters&) {}
    /// Called during the upload pass
    virtual void upload(gfx::Context&, gfx::UploadPass&) {}
    /// Called during each render pass
    virtual void render(RenderOrchestrator&, PaintParameters&) {}
    /// Called at the end of each frame
    virtual void postRender(RenderOrchestrator&, PaintParameters&) {}

    /// Call the provided function for each drawable in undefined order
    virtual void observeDrawables(std::function<void(gfx::Drawable&)>) = 0;
    virtual void observeDrawables(std::function<void(const gfx::Drawable&)>) const = 0;

    /// Call the provided function for each drawable in undefined order, allowing ownership to be taken.
    virtual void observeDrawables(std::function<void(gfx::UniqueDrawable&)>) = 0;

protected:
    int32_t layerIndex;
};

/**
    A layer group for tile-based drawables.
 */
class TileLayerGroup : public LayerGroup {
public:
    TileLayerGroup(int32_t layerIndex, std::size_t initialCapacity);
    ~TileLayerGroup() override;

    std::size_t getDrawableCount() const;

    const gfx::UniqueDrawable& getDrawable(mbgl::RenderPass, const OverscaledTileID&) const;
    gfx::UniqueDrawable removeDrawable(mbgl::RenderPass, const OverscaledTileID&);
    bool addDrawable(mbgl::RenderPass, const OverscaledTileID&, gfx::UniqueDrawable&&);

    void observeDrawables(std::function<void(gfx::Drawable&)>) override;
    void observeDrawables(std::function<void(const gfx::Drawable&)>) const override;
    void observeDrawables(std::function<void(gfx::UniqueDrawable&)>) override;

    void clearDrawables();

protected:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace mbgl
