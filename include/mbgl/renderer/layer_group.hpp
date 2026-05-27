#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/identity.hpp>
#include <mbgl/util/logging.hpp>

#include <functional>
#include <memory>
#include <string>
#include <set>
#include <vector>

namespace mbgl {
class LayerGroupBase;
class PaintParameters;
class RenderOrchestrator;
class RenderTile;
class RenderTree;
class TileLayerGroup;

using LayerGroupBasePtr = std::shared_ptr<LayerGroupBase>;
using RenderTiles = std::shared_ptr<const std::vector<std::reference_wrapper<const RenderTile>>>;

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
public:
    enum class Type : uint8_t {
        LayerGroup = 0,
        TileLayerGroup,

        Invalid = 255
    };

protected:
    LayerGroupBase(int32_t layerIndex, std::string name = std::string(), Type type = Type::Invalid);

public:
    LayerGroupBase(const LayerGroupBase&) = delete;
    LayerGroupBase& operator=(const LayerGroupBase&) = delete;
    ~LayerGroupBase() override = default;

    /// Whether the drawables should be drawn
    bool getEnabled() const { return enabled; }
    void setEnabled(bool value) { enabled = value; }

    /// Get the type of layer group
    Type getType() const noexcept { return type; }

    /// Get the name of the layer group
    const std::string& getName() const { return name; }
    /// Set the name of the layer group
    void setName(std::string value) { name = std::move(value); }

    /// Get the layer index
    int32_t getLayerIndex() const { return layerIndex; }

    /// Update the layer index to a new value
    virtual void updateLayerIndex(int32_t value) { layerIndex = value; }

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

    /// Attach a tweaker to be run on this layer group for each frame.
    /// Tweaker lifetime is controlled by the render layer and drawables, the layer group retains a weak reference.
    void addLayerTweaker(const LayerTweakerPtr& tweaker) { layerTweakers.emplace_back(tweaker); }

    void runTweakers(const RenderTree&, PaintParameters&);

    /// Get the uniform buffers attached to this layer group
    virtual const gfx::UniformBufferArray& getUniformBuffers() const = 0;

    /// Get the mutable uniform buffer array attached to this layer group
    virtual gfx::UniformBufferArray& mutableUniformBuffers() = 0;

protected:
    const Type type;
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

    template <typename Func /* void(gfx::Drawable&) */>
    std::size_t visitDrawables(Func f) {
        assert(drawablesByTile.size() == sortedDrawables.size());
        for (auto* drawable : sortedDrawables) {
            f(*drawable);
        }
        return sortedDrawables.size();
    }

    template <typename Func /* bool(gfx::Drawable&) */>
    std::size_t removeDrawablesIf(Func f) {
        const auto oldSize = drawablesByTile.size();
        for (auto i = drawablesByTile.begin(); i != drawablesByTile.end();) {
            auto& drawable = i->second;
            if (!f(*drawable)) {
                // Not removed, keep going
                ++i;
            } else {
                // Removed, take it out of the collections
                sortedDrawables.erase(drawable.get());
                i = drawablesByTile.erase(i);
            }
            assert(drawablesByTile.size() == sortedDrawables.size());
        }
        return (oldSize - drawablesByTile.size());
    }

    /// Call the provided function for each drawable for the given tile
    template <typename Func /* void(gfx::Drawable&) */>
    std::size_t visitDrawables(mbgl::RenderPass pass, const OverscaledTileID& tileID, Func f) {
        assert(drawablesByTile.size() == sortedDrawables.size());
        const auto range = drawablesByTile.equal_range({pass, tileID});
        std::for_each(range.first, range.second, [&f](const auto& pair) { f(*pair.second); });
        return std::distance(range.first, range.second);
    }

    std::size_t clearDrawables() override;

    void setStencilTiles(RenderTiles);

    void updateLayerIndex(int32_t value) override { layerIndex = value; }

protected:
    // When stencil clipping is enabled for the layer, this is the set
    // of tile IDs that need to be rendered to the stencil buffer.
    RenderTiles stencilTiles;

    struct TileLayerGroupTileKey {
        mbgl::RenderPass renderPass;
        OverscaledTileID tileID;
        bool operator==(const TileLayerGroupTileKey& other) const {
            return renderPass == other.renderPass && tileID == other.tileID;
        }
        struct hash {
            size_t operator()(const TileLayerGroupTileKey& k) const {
                return (std::hash<mbgl::RenderPass>()(k.renderPass) ^ std::hash<OverscaledTileID>()(k.tileID) << 1);
            }
        };
    };

private:
    using TileMap = std::unordered_multimap<TileLayerGroupTileKey, gfx::UniqueDrawable, TileLayerGroupTileKey::hash>;
    TileMap drawablesByTile;

    using DrawableMap = std::set<gfx::Drawable*, gfx::DrawableLessByPriority>;
    DrawableMap sortedDrawables;
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

    template <typename Func /* void(gfx::Drawable&) */>
    std::size_t visitDrawables(Func f) {
        for (const auto& item : drawables) {
            if (item) {
                f(*item);
            }
        }
        return drawables.size();
    }

    template <typename Func /* bool(gfx::Drawable&) */>
    std::size_t removeDrawablesIf(Func f) {
        decltype(drawables) newSet;
        const auto oldSize = drawables.size();
        while (!drawables.empty()) {
            // set members are immutable, since changes could affect its position, so extract each item
            gfx::UniqueDrawable drawable = std::move(drawables.extract(drawables.begin()).value());
            if (!f(*drawable)) {
                // Not removed, keep it, but in a new set so that if the key value
                // has increased, we don't see it again during this iteration.
                newSet.emplace_hint(newSet.end(), std::move(drawable));
            }
        }
        std::swap(drawables, newSet);
        return (oldSize - drawables.size());
    }

    std::size_t clearDrawables() override;

    void updateLayerIndex(int32_t value) override { layerIndex = value; }

protected:
    using DrawableCollection = std::set<gfx::UniqueDrawable, gfx::DrawableLessByPriority>;
    DrawableCollection drawables;
};

template <typename Func /* void(gfx::Drawable&) */>
void visitLayerGroupDrawables(mbgl::LayerGroupBase& layerGroup, Func dg) {
    switch (layerGroup.getType()) {
        case LayerGroupBase::Type::LayerGroup: {
            static_cast<LayerGroup&>(layerGroup).visitDrawables(dg);
            break;
        }
        case LayerGroupBase::Type::TileLayerGroup: {
            static_cast<TileLayerGroup&>(layerGroup).visitDrawables(dg);
            break;
        }
        default: {
#ifndef NDEBUG
            mbgl::Log::Error(mbgl::Event::Render,
                             "Unknown layer group type: " + std::to_string(static_cast<uint8_t>(layerGroup.getType())));
#endif
            break;
        }
    }
}

template <typename Func /* bool(gfx::Drawable&) */>
std::size_t removeLayerGroupDrawablesIf(mbgl::LayerGroupBase& layerGroup, Func dg) {
    switch (layerGroup.getType()) {
        case LayerGroupBase::Type::LayerGroup: {
            return static_cast<LayerGroup&>(layerGroup).removeDrawablesIf(dg);
        }
        case LayerGroupBase::Type::TileLayerGroup: {
            return static_cast<TileLayerGroup&>(layerGroup).removeDrawablesIf(dg);
        }
        default: {
#ifndef NDEBUG
            mbgl::Log::Error(mbgl::Event::Render,
                             "Unknown layer group type: " + std::to_string(static_cast<uint8_t>(layerGroup.getType())));
#endif
            return 0;
        }
    }
}

} // namespace mbgl
