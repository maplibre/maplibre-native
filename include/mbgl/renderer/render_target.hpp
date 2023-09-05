#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace mbgl {

namespace gfx {
class Texture2D;
class UploadPass;
using Texture2DPtr = std::shared_ptr<Texture2D>;
} // namespace gfx

class LayerGroupBase;
class PaintParameters;
class RenderOrchestrator;
class RenderTree;

using LayerGroupBasePtr = std::shared_ptr<LayerGroupBase>;

/// Render target class
class RenderTarget {
public:
    virtual ~RenderTarget() = default;

    /// Set the render target texture
    void setTexture(const gfx::Texture2DPtr& texture_) { texture = std::move(texture_); };

    /// Get the render target texture
    const gfx::Texture2DPtr& getTexture() const { return texture; };

    /// @brief Add a layer group to the render target
    /// @param replace Flag to replace if exists
    /// @return whether added
    bool addLayerGroup(LayerGroupBasePtr, bool replace);

    /// @brief Remove a layer group
    /// @param layerIndex index of the layer to remove
    /// @return whether removed
    bool removeLayerGroup(const int32_t layerIndex);

    /// Get the layer group count
    size_t numLayerGroups() const noexcept;

    /// @brief  Get a specific layer group by index
    /// @param layerIndex index
    /// @return the layer group if existant, othewise a shared null pointer
    const LayerGroupBasePtr& getLayerGroup(const int32_t layerIndex) const;

    /// Execute the given function for each contained layer group
    void visitLayerGroups(std::function<void(LayerGroupBase&)>);
    void visitLayerGroups(std::function<void(const LayerGroupBase&)>) const;

    /// Upload the layer groups
    virtual void upload(gfx::UploadPass& uploadPass) = 0;

    /// Render the layer groups
    virtual void render(RenderOrchestrator&, const RenderTree&, PaintParameters&) = 0;

protected:
    gfx::Texture2DPtr texture;
    using LayerGroupMap = std::map<int32_t, LayerGroupBasePtr>;
    LayerGroupMap layerGroupsByLayerIndex;
};

} // namespace mbgl
