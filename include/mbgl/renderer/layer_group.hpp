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
class RenderTree;
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
using LayerTweakerWeakPtr = std::weak_ptr<LayerTweaker>;

/**
    A layer-like group of drawables, not a group of layers.
 */
class LayerGroupBase : public util::SimpleIdentifiable {
protected:
    LayerGroupBase(int32_t layerIndex, std::string name = std::string());

public:
    LayerGroupBase(const LayerGroupBase&) = delete;
    LayerGroupBase& operator=(const LayerGroupBase&) = delete;
    ~LayerGroupBase() override = default;

    /// Whether the drawables should be drawn
    bool getEnabled() const { return enabled; }
    void setEnabled(bool value) { enabled = value; }

    /// Get the name of the layer group
    const std::string& getName() const { return name; }
    /// Set the name of the layer group
    void setName(std::string value) { name = std::move(value); }

    /// Get the layer index
    int32_t getLayerIndex() const { return layerIndex; }

    /// Update the layer index to a new value
    void updateLayerIndex(int32_t value) { layerIndex = value; }

    /// Get the number of drawables contained
    virtual std::size_t getDrawableCount() const = 0;

    /// Whether the number of drawables contained is zero
    bool empty() const { return getDrawableCount() == 0; }

    /// Clear the drawable collection
    virtual std::size_t clearDrawables() = 0;

    /// Add a drawable
    void addDrawable(gfx::UniqueDrawable&);

    /// Called before starting each frame
    virtual void preRender(RenderOrchestrator&, PaintParameters&) {}
    /// Called during the upload pass
    virtual void upload(gfx::UploadPass&) {}
    /// Called during each render pass
    virtual void render(RenderOrchestrator&, PaintParameters&) {}
    /// Called at the end of each frame
    virtual void postRender(RenderOrchestrator&, PaintParameters&) {}

    /// Call the provided function for each drawable in priority order
    virtual std::size_t visitDrawables(const std::function<void(gfx::Drawable&)>&&) = 0;
    virtual std::size_t visitDrawables(const std::function<void(const gfx::Drawable&)>&&) const = 0;

    /// Call the provided function for each drawable in undefined order, allowing for removal.
    /// @param f A function called with each drawable, returning true to discard it and false to keep it
    /// @return The number of items removed
    virtual std::size_t removeDrawablesIf(const std::function<bool(gfx::Drawable&)>&& f) = 0;

    /// Attach a tweaker to be run on this layer group for each frame.
    /// Tweaker lifetime is controlled by the render layer and drawables, the layer group retains a weak reference.
    void addLayerTweaker(const LayerTweakerPtr& tweaker) { layerTweakers.emplace_back(tweaker); }

    void runTweakers(const RenderTree&, PaintParameters&);

protected:
    bool enabled = true;
    int32_t layerIndex;
    std::vector<LayerTweakerWeakPtr> layerTweakers;
    std::string name;
};

/**
    A layer group for tile-based drawables.
 */
class TileLayerGroup : public LayerGroupBase {
public:
    TileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name);
    ~TileLayerGroup() override;

    std::size_t getDrawableCount() const override;
    std::size_t getDrawableCount(mbgl::RenderPass, const OverscaledTileID&) const;

    std::vector<gfx::UniqueDrawable> removeDrawables(mbgl::RenderPass, const OverscaledTileID&);
    void addDrawable(mbgl::RenderPass, const OverscaledTileID&, gfx::UniqueDrawable&&);

    std::size_t visitDrawables(const std::function<void(gfx::Drawable&)>&&) override;
    std::size_t visitDrawables(const std::function<void(const gfx::Drawable&)>&&) const override;
    std::size_t removeDrawablesIf(const std::function<bool(gfx::Drawable&)>&&) override;

    /// Call the provided function for each drawable for the given tile
    std::size_t visitDrawables(mbgl::RenderPass, const OverscaledTileID&, const std::function<void(gfx::Drawable&)>&&);
    std::size_t visitDrawables(mbgl::RenderPass,
                               const OverscaledTileID&,
                               const std::function<void(const gfx::Drawable&)>&&) const;

    std::size_t clearDrawables() override;

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

    std::size_t getDrawableCount() const override;
    std::size_t getDrawableCount(mbgl::RenderPass) const;

    std::vector<gfx::UniqueDrawable> removeDrawables(mbgl::RenderPass);
    void addDrawable(gfx::UniqueDrawable&&);

    std::size_t visitDrawables(const std::function<void(gfx::Drawable&)>&&) override;
    std::size_t visitDrawables(const std::function<void(const gfx::Drawable&)>&&) const override;
    std::size_t removeDrawablesIf(const std::function<bool(gfx::Drawable&)>&&) override;

    std::size_t clearDrawables() override;

protected:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace mbgl
