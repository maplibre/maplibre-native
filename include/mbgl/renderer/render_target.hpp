#pragma once

#include <mbgl/gfx/types.hpp>
#include <mbgl/util/size.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace mbgl {

namespace gfx {
class Context;
class Texture2D;
class OffscreenTexture;
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
    RenderTarget(gfx::Context& context, const Size size, const gfx::TextureChannelDataType type);
    ~RenderTarget();

    /// Get the render target texture
    const gfx::Texture2DPtr& getTexture();

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
    void upload(gfx::UploadPass& uploadPass);

    /// Render the layer groups
    void render(RenderOrchestrator&, const RenderTree&, PaintParameters&);

protected:
    gfx::Context& context;
    std::unique_ptr<gfx::OffscreenTexture> offscreenTexture;
    using LayerGroupMap = std::map<int32_t, LayerGroupBasePtr>;
    LayerGroupMap layerGroupsByLayerIndex;
};

} // namespace mbgl
