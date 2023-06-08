#pragma once

#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/identity.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace mbgl {
class LayerGroupBase;
class PaintParameters;
class RenderOrchestrator;
class TileLayerGroup;

using LayerGroupBasePtr = std::shared_ptr<LayerGroupBase>;

namespace gfx {
class Context;
class Drawable;
class UploadPass;

using DrawablePtr = std::shared_ptr<Drawable>;
using UniqueDrawable = std::unique_ptr<Drawable>;
} // namespace gfx

class LayerTweaker;
using LayerTweakerPtr = std::shared_ptr<LayerTweaker>;

/**
    A layer-like group of drawables, not a group of layers.
 */
class LayerGroupBase : public util::SimpleIdentifiable {
protected:
    LayerGroupBase(int32_t layerIndex, std::string name = std::string());

public:
    LayerGroupBase(const LayerGroupBase&) = delete;
    LayerGroupBase& operator=(const LayerGroupBase&) = delete;
    virtual ~LayerGroupBase() = default;

    /// Whether the drawables should be drawn
    bool getEnabled() const { return enabled; }
    void setEnabled(bool value) { enabled = value; }

    const std::string& getName() const { return name; }
    void setName(std::string value) { name = std::move(value); }

    int32_t getLayerIndex() const { return layerIndex; }

    virtual std::size_t getDrawableCount() const = 0;
    bool empty() const { return getDrawableCount() == 0; }

    /// Called before starting each frame
    virtual void preRender(RenderOrchestrator&, PaintParameters&) {}
    /// Called during the upload pass
    virtual void upload(gfx::UploadPass&) {}
    /// Called during each render pass
    virtual void render(RenderOrchestrator&, PaintParameters&) {}
    /// Called at the end of each frame
    virtual void postRender(RenderOrchestrator&, PaintParameters&) {}

    /// Call the provided function for each drawable in undefined order
    virtual void observeDrawables(std::function<void(gfx::Drawable&)>) = 0;
    virtual void observeDrawables(std::function<void(const gfx::Drawable&)>) const = 0;

    /// Call the provided function for each drawable in undefined order, allowing ownership to be taken.
    virtual void observeDrawables(std::function<void(gfx::UniqueDrawable&)>) = 0;

    /// Attach a tweaker to be run on this layer group for each frame
    void setLayerTweaker(LayerTweakerPtr tweaker) { layerTweaker = std::move(tweaker); }

    /// Get the tweaker attached to this layer group
    const LayerTweakerPtr& getLayerTweaker() const { return layerTweaker; }

protected:
    bool enabled = true;
    int32_t layerIndex;
    LayerTweakerPtr layerTweaker;
    std::string name;
};

/**
    A layer group for tile-based drawables.
 */
class TileLayerGroup : public LayerGroupBase {
public:
    TileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name);
    ~TileLayerGroup() override;

    void updateLayerIndex(int32_t newLayerIndex);

    std::size_t getDrawableCount() const override;
    std::size_t getDrawableCount(mbgl::RenderPass, const OverscaledTileID&) const;

    std::vector<gfx::UniqueDrawable> removeDrawables(mbgl::RenderPass, const OverscaledTileID&);
    void addDrawable(mbgl::RenderPass, const OverscaledTileID&, gfx::UniqueDrawable&&);

    void observeDrawables(std::function<void(gfx::Drawable&)>) override;
    void observeDrawables(std::function<void(const gfx::Drawable&)>) const override;
    void observeDrawables(std::function<void(gfx::UniqueDrawable&)>) override;

    /// Call the provided function for each drawable for the given tile
    void observeDrawables(mbgl::RenderPass, const OverscaledTileID&, std::function<void(const gfx::Drawable&)>) const;

    std::size_t clearDrawables();

protected:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

/**
    A layer group for non-tile-based drawables.
 */
class LayerGroup : public LayerGroupBase {
public:
    LayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name);
    ~LayerGroup() override;

    void updateLayerIndex(int32_t newLayerIndex);

    std::size_t getDrawableCount() const override;
    std::size_t getDrawableCount(mbgl::RenderPass) const;

    std::vector<gfx::UniqueDrawable> removeDrawables(mbgl::RenderPass);
    void addDrawable(gfx::UniqueDrawable&&);

    void observeDrawables(std::function<void(gfx::Drawable&)>) override;
    void observeDrawables(std::function<void(const gfx::Drawable&)>) const override;
    void observeDrawables(std::function<void(gfx::UniqueDrawable&)>) override;

    std::size_t clearDrawables();

protected:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace mbgl
